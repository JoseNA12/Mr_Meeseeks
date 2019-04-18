#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <math.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#define SIZE 256 // Tamaño de la variable para recibir el input del usuario

float TIEMPOMAX = 300.0; // Tiempo máximo antes de que todos los Mr. Meeseeks entren en un caos planetario
#define TIEMPOMAXREQ 5 // Tiempo máximo en responder una petición
#define TIEMPOMINREQ 0.5 // Tiempo mínimo en responder una petición
// Parametros de la distribucion normal
#define MEDIA 0
#define VARIANZA 1
#define RANGO 1
#define CANTMUESTRAS 10000

#define DIFICULTADMAX 100
#define DIFICULTADMIN 0


// Nivel de forks e instancias
int N = 0;
int I = 0;

// [3]: google-chrome, geany, atom, pinta
// Compilar con: gcc main.c -o main -lm -pthread


// http://cypascal.blogspot.com/2016/02/crear-una-distribucion-normal-en-c.html
float distribucionNormal(){
    srand(time(NULL));  // Reiniciar la semilla de rand()
    int i = 1; float aux;

    for(i; i <= CANTMUESTRAS; i++){
        aux += (float)rand()/RAND_MAX;
    }
    return fabs(VARIANZA * sqrt((float)12/CANTMUESTRAS) * (aux - (float)CANTMUESTRAS/2) + MEDIA) * RANGO;
}

// Obtener un tiempo aleatorio entre en rango, máximo y mínimo
float getNumDistrNormal(int max, int min) {
    srand(time(NULL));  // Reiniciar la semilla de rand()
    return rand() % (max + 1 - min) + min;
}

// Obtener un tiempo basado en la dificultad
float setTiempoTarea(float dificultad) {
    float tiempo = (100 - dificultad) / 100 * TIEMPOMAXREQ;
    if (TIEMPOMINREQ > tiempo){
        tiempo = TIEMPOMINREQ;
    }
    return tiempo;
}

// Determinar la cantidad de Mr Meeseeks a crear
int determinarHijos(float dificultad) {
    if (dificultad >= 85.01) { // 100 - 85.01 = 0 hijos
        return 0;
    } 
    else {
        if (85 >= dificultad && dificultad > 45){ // 85 - 45.01 = min 1 hijo
            return getNumDistrNormal(45, 1);
        } else { // 45 - 0 = min 3 hijos
            return getNumDistrNormal(85, 3);
        }
    }
}

// https://bytefreaks.net/programming-2/c-programming-2/cc-pass-value-from-parent-to-child-after-fork-via-a-pipe
// Encargado de crear un pipe entre procesos padre e hijo
void comunicarProcesos(int fd[2], pid_t pid, char mensaje[SIZE]) {
    char *leerMensaje;

    if (pid != 0) { // padre
        close(fd[0]); // solo escritura

        write(fd[1], &mensaje, sizeof(&mensaje)); // Enviar el mensaje en el descriptor de escritura.
        //printf("Padre (%d) envia envia: %s\n", getpid(), mensaje);

        close(fd[1]); // cierra el descriptor de escritura
    }
    else {
        close(fd[1]); // solo lectura, cierra el descriptor de escritura

        // Ahora lee los datos (se bloqueará hasta que tenga éxito, sincrónico me parece)
        read(fd[0], &leerMensaje, sizeof(leerMensaje));
        printf("Mr. Meeseeks (%d, %d): He recibido la petición '%s'", getpid(), getppid(), leerMensaje);

        close(fd[0]); // cerrar el descriptor de lectura
    }
}

int* crearPipe() { 
    static int fd[2]; 
    int retornoPipe;
    
    retornoPipe = pipe(fd); // crear descriptores de tubería

    if (retornoPipe == -1) {
        printf("No se ha podido crear el pipe\n");
    }
    return fd;
}

pid_t crearFork(char peticion[SIZE]) {
    int* fd = crearPipe();
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Error al crear el Mr Meeseek :(\n");
    } 
    else if (pid == 0) { // Proceso hijo
        printf("\nHi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n", 
            getpid(), getppid(), N, I);
    }
    // Comunicacion entre 2 procesos mediante un pipe
    comunicarProcesos(fd, pid, peticion);

    return pid;
}

// funcion para obtener mensajes random de espera en consola
char* mensajeEspera() {
    int limite = 6; // de 0 a 6 digitos
	time_t t;
    srand((unsigned) time(&t)); // reiniciar semilla del srand()

    int msg = (rand() % (limite + 1));
    switch (msg) {
    case 0: return "Dejame pensar en tu tarea...";
    case 1: return "Espera, parece que tiene solución...";
    case 2: return "No se ve complicado, dejame pensar...";
    case 3: return "Espera, creo que esto lo he hecho antes...";
    case 4: return "Me estoy esforzando para hacer tu tarea, espera...";
    case 5: return "Estoy pensando en la resolución de tu tarea, espera...";
    case 6: return "Pienso lo más rapido que puedo, un momento...";
    default: break;
    }
}

void consultaTextual() {
    char peticion[SIZE], respuesta;
    float dificultad, tiempo_tarea;
    int tieneDificultad = 0;  // determinar si el usuario ingresó una dificultad
    pid_t procesoBoxMrMeeseek = getpid(); // proceso original (sin haber creado ningun mr meeseek)

    // variables del tiempo
    time_t tiempoInicio, tiempoActual, tiempoLocal_1, tiempoLocal_2;
    struct tm *structInicio;
    struct tm structActual;
    struct tm strucTiempoLocal_1;
    struct tm strucTiempoLocal_2;

    // Consultas al usuario
    printf("\nEscribe tu petición:\n>>> ");
    scanf("%s", peticion);
    printf("\n¿Conocés la dificultad de tu petición? [y/n]:\n>>> ");
    scanf(" %c", &respuesta);

    if (respuesta == 'y') {
        printf("\nRango de 0 a 100: >>> ");
        scanf("%f", &dificultad);
        tieneDificultad = 1;
    }
    else {
        dificultad = distribucionNormal() * getNumDistrNormal(DIFICULTADMAX, DIFICULTADMIN);
    }

    // Definir la hora de inicio de la tarea
    time(&tiempoInicio); // Segundos que han pasado desde January 1, 1970
    structInicio = localtime(&tiempoInicio); // Transforma esos segundos en la fecha y hora actual

    // Crear el primer Mr Meeseek
    pid_t primerMrMeekseek = crearFork(peticion);

    tiempoActual = time(NULL);
    structActual = *((struct tm*)localtime(&tiempoActual));

    if (primerMrMeekseek == 0) { // si es hijo
        printf("\nMr. Meeseeks (%d, %d): Dificultad de %f%%", getpid(), getppid(), dificultad);

        while (difftime(tiempoActual, tiempoInicio) < TIEMPOMAX) { // empieza a contar el tiempo
            tiempo_tarea = setTiempoTarea(dificultad); // tiempo en que el Mr M responderá a la solicitud
            
            tiempoLocal_1 = time(NULL);
            strucTiempoLocal_1 = *((struct tm*)localtime(&tiempoLocal_1));

            printf("\nMr. Meeseeks (%d, %d): ", getpid(), getppid()); // mostrar un msg de espera aleatorio
            printf("%s", mensajeEspera());

            while (difftime(tiempoLocal_2, tiempoLocal_1) <= tiempo_tarea) { // el Mr M "piensa"
                
                // DISMINUIR la dificultad
                tiempoActual = time(NULL); // ir actualizando el tiempo
                structActual = *((struct tm*)localtime(&tiempoActual));

                tiempoLocal_2 = time(NULL);
                strucTiempoLocal_2 = *((struct tm*)localtime(&tiempoLocal_2));
            }

            // meter un if para comparar la dificultad y ver si se necesitan ayudantes

            int ayudantesMrM = determinarHijos(dificultad); // Determinar si el Mr M necesita ayuda

            printf("%d ", ayudantesMrM);

            if (ayudantesMrM > 0) {
                printf("\nMr. Meeseeks (%d, %d): Necesitaré ayuda!. Me multiplicaré %d veces", 
                        getpid(), getppid(), ayudantesMrM);

                break;
                // meter semaforos, manejar la variable compartida "i" de numero de instancias
                // meter condición de parada para terminar la tarea segun la dificultad diluida
                // pipe entre el proceso que finalizó la tarea y el padre para hacer la terminación de la tarea
            }
            else { // 0 ayudantes
                // - terminar la tarea
                // - comunicar con pipes al proceso padre que alguien la resolvió
            }
        }
        //kill(getpid(), SIGTERM); // Eliminar el proceso hijo
    }
    else {
        wait(NULL); // esperar a que el Mr Meeseeks resuelva la tarea
    }
    
}

// Calculo matematico de operaciones binarias
int calculoMatematico(char hileras[SIZE]) {
    int a,b;
    char op;
    int resultado; 
    if (sscanf(hileras, "%d %c %d", &a, &op, &b) != 3) { // parsear el string y obtener los valores
        printf("\nMr. Meeseeks (%d, %d): Formato inválido de las hileras\n", getpid(), getppid());
        return resultado;
    }
    else {
        switch (op) {
            case '+': resultado = a + b; break;
            case '-': resultado = a - b; break;
            case '*': resultado = a * b; break;
            case '/': resultado = a / b; break;
            default:
                printf("\nMr. Meeseeks (%d, %d): Operador de las hileras inválido\n", getpid(), getppid());
        }
        return resultado;
    }
}

int ejecutarPrograma(char path[SIZE]) {
    return system(path); // valor de retorno del programa ejecutado (0 o 1)
}

void box_Mr_Meeseeks() {
    while(1) {
        printf("\n\n======================== Box Mr.Meeseeks ========================\n");
        printf("        [1] - Consulta textual\n");
        printf("        [2] - Cálculo matemático\n");
        printf("        [3] - Ejecutar un programa\n\n");
        printf("        [4] - Tiempo máximo para el caos planetario\n");
        printf("        [5] - Consultar información de la Box Mr.Meeseeks");
        printf("\n=================================================================\n\n");
        printf("Esperando una solicitud: ");

        int solicitud;
        scanf("%d", &solicitud);

        if (solicitud == 1) { // Consulta textual
            consultaTextual();
        }
        else if (solicitud == 2) // Cálculo matemático
        {
            char hileras[SIZE];
            printf("\nIngrese la hilera matemática:\n>>> ");
            scanf("%s", hileras);

            pid_t pid = crearFork(strcat(hileras, " -> [Cálculo matemático]" ));

            if (pid == 0) {
                int resultado = calculoMatematico(hileras);

                printf("\nMr. Meeseeks (%d, %d): El resultado es: %d\n", getpid(), getppid(), resultado);
                printf("Mr. Meeseeks (%d, %d): Fue un placer servirle. Adios!", getpid(), getppid());
                kill(getpid(), SIGTERM); // Eliminar el proceso hijo
            }
            else {
                wait(NULL); // Esperar que el proceso hijo complete su tarea
            }
        }
        else if (solicitud == 3) // Ejecutar un programa
        {
            char path[SIZE];
            printf("\nIngrese el path/comando del programa:\n>>> ");
            scanf("%s", path);

            pid_t pid = crearFork(strcat(path, " -> [Ejecutar un programa]"));

            if (pid == 0) {
                int status = ejecutarPrograma(path);

                switch (status) {
                    case -1:
                        printf("\nMr. Meeseeks (%d, %d): Error al ejecutar el programa ingresado!\n", getpid(), getppid());
                        break;
                    
                    case 0:
                        printf("\nMr. Meeseeks (%d, %d): El programa se ha ejecutado!\n", getpid(), getppid());
                        break;
                        
                    default:
                        break;
                }
                kill(getpid(), SIGTERM); // Eliminar el proceso hijo
            }
            else {
                wait(NULL); // Esperar que el proceso hijo complete su tarea
            }
        }
        else if (solicitud == 4) { // Configurar tiempo máximo caos planetario
            printf("\n- Tiempo actual: %lf segundos\n", TIEMPOMAX);
            printf("¿Desea cambiar el tiempo? [y/n]\n>>> ");
            char respuesta;
            scanf(" %c", &respuesta);

            if (respuesta == 'y') {
                printf("\nIngrese el valor (en segundos):\n>>> ");
                scanf("%f", &TIEMPOMAX);
            }
            printf("\nTiempo establecido: %lf \n", TIEMPOMAX);
        }
        else if (solicitud == 5) {
            printf("\nArroz con pollo");
        }
        else if (solicitud == -1)
        {
            printf("\n*** La Box Mr.Meeseeks ha sido destruida! ***\n");
            break;
        }
        else
        {
            printf("\n*** Opción no válida ***");
        }
    }
}

int main(){
    box_Mr_Meeseeks();
}