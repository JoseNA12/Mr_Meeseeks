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
#include <sys/mman.h>
#include "Vector.c"

#include "MainForks_.h"

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

static int *solucionado; // variable que define la solucion de una tarea
static int *pidPrimerMrM; // almacenar el pid del Mr M hijo en caso de declarar caos y asi poder eliminarlo
static datos_compartidos* datos = NULL; // variables para el mutex lock
static vector *lista_procesos;

static int* fd; // Pipe Global

// Nivel de forks e instancias
int N = 1;
//int I = 0;

// [3]: google-chrome, geany, atom, pinta
// Compilar con: gcc main.c -o main -lm -pthread


// Encargado de inicializar los datos de la memoria compartida entre procesos
void initDatosCompartidos() {
  
    int prot = PROT_READ | PROT_WRITE;
    int flags = MAP_SHARED | MAP_ANONYMOUS;
    datos = mmap(NULL, sizeof(datos_compartidos), prot, flags, -1, 0);
    
    // inicializa la memoria compartida
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&datos->mutex, &attr);
}

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
float setTiempoSolicitud(float dificultad) {
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
    static int fd_temp[2]; 
    int retornoPipe;
    
    retornoPipe = pipe(fd_temp); // crear descriptores de tubería

    if (retornoPipe == -1) {
        printf("No se ha podido crear el pipe\n");
    }
    return fd_temp;
}

pid_t crearFork(char peticion[SIZE], int I) {
    fd = crearPipe();
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

void mensajeBomba() {
    printf("\n");
    printf("\n         _.-^^---....,,--       ");
    printf("\n     _--                  --_   ");
    printf("\n    <                        >) ");
    printf("\n    |                         | ");
    printf("\n     -._                   _./  ");
    printf("\n        ```--. . , ; .--''' ");
    printf("\n              | |   |           ");
    printf("\n           .-=||  | |=-.        ");
    printf("\n           `-=#$-&-$#=-'        ");
    printf("\n              | ;  :|           ");
    printf("\n     _____.,-#*&$@*#&#~,._____  \n");
}

void consultaTextual() {
    char peticion[SIZE], respuesta;
    float dificultad, tiempo_solicitud;
    pid_t procesoBoxMrMeeseek = getpid(); // proceso original (sin haber creado ningun mr meeseek)

    // Datos de memoria compartida
    solucionado = mmap(NULL, sizeof *solucionado, PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0); // Crear la variable compartida
	*solucionado = 0; // 0 -> sin solucion, 1 -> solucionado
    
    // pid del primer Mr M en caso de declarar caos para eliminarlo
    pidPrimerMrM = mmap(NULL, sizeof *pidPrimerMrM, PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0); // Crear la variable compartida
    *pidPrimerMrM = -1;

    lista_procesos = mmap(NULL, sizeof *lista_procesos, PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0); // Crear la variable compartida

    vector_init(lista_procesos);

    initDatosCompartidos(); // inicializar memoria compartida

    // Consultas al usuario
    printf("\nEscribe tu petición:\n>>> ");
    scanf("%s", peticion);
    printf("\n¿Conocés la dificultad de tu petición? [y/n]:\n>>> ");
    scanf(" %c", &respuesta);

    if (respuesta == 'y') {
        printf("\nRango de 0 a 100: >>> ");
        scanf("%f", &dificultad);
    }
    else {
        dificultad = distribucionNormal() * getNumDistrNormal(DIFICULTADMAX, DIFICULTADMIN);
    }

    // Crear el primer Mr Meeseek
    // Crear el primer Mr Meeseek
    pid_t mrMeekseek = crearFork(peticion, 1);
    //pid_t *mrMeeseekAyudante;

    if (mrMeekseek == 0) { // si es hijo
        *pidPrimerMrM = getpid(); // guardar el pid del 1er Mr M en caso de caos planetario para que el proceso padre pueda matarlo
        vector_free(lista_procesos); // limpiar la lista
        vector_init(lista_procesos); // inicializa la lista

        while (!*solucionado) { // verificar si se ha solucionado la solicitud

            tiempo_solicitud = setTiempoSolicitud(dificultad);
            printf("\nMr. Meeseeks (%d, %d): Dificultad de %f%%", getpid(), getppid(), dificultad);
            printf("\nMr. Meeseeks (%d, %d): ", getpid(), getppid()); // mostrar un msg de espera aleatorio
            printf("%s. (%f seg)", mensajeEspera(), tiempo_solicitud); // este hp mensaje no se porqué lo imprime despues que "piensa"

            // Mr Meeseeks "piensa"
            sleep(tiempo_solicitud);

            if (dificultad > 85.01) {
                pthread_mutex_lock(&datos->mutex); // bloquear el recurso compartido
                if (!*solucionado) {
                    /// comunicar a los hijos que se encontró
                                        
                    int msg = getpid();
                    close(fd[0]); // solo escritura
                    write(fd[1], &msg, sizeof(msg)); // Enviar el mensaje en el descriptor de escritura.
                    close(fd[1]); // cierra el descriptor de escritura

                    //comunicarProcesos(fd, *mrMeeseekAyudante, "\nSolicitud terminada!");

                    *solucionado = 1;
                }
                pthread_mutex_unlock(&datos->mutex); // liberar el recurso compartido
            }
            else {
                int ayudantesMrM = determinarHijos(dificultad); // Determinar si el Mr M necesita ayuda

                if (ayudantesMrM > 0) {
                    printf("\nMr. Meeseeks (%d, %d): Necesitaré ayuda!. Me multiplicaré %d veces", 
                            getpid(), getppid(), ayudantesMrM);

                    N++;
                    dificultad = getNumDistrNormal(DIFICULTADMAX, dificultad); // disminuir la dificultad para los hijos
                    for (int I = 1; I <= 3; I++) {
                        //mrMeeseekAyudante = (pid_t*)malloc(sizeof(pid_t));
                        //*mrMeeseekAyudante = crearFork(peticion, I);
                        mrMeekseek = crearFork(peticion, I);

                        if (mrMeekseek == 0) { // solo el padre crea hijos
                            break; 
                        } 
                        else { // si es padre, registre a sus hijos
                            if (mrMeekseek != -1) {
                                vector_add(lista_procesos, &mrMeekseek);
                            } 
                            else { 
                                break;
                            }
                        }
                    }
                }
            }
        }
        // revisar si el primer Mr M no tiene hijos
        if (vector_total(lista_procesos) == 0 ) {
            printf("\nMr. Meeseeks (%d, %d): Yei lo hice solo, adios un placer ayudarte!", 
                            getpid(), getppid());
        } 
        else { // esperar a todos los Mr M hijos creados
            for (int i = 0; i < vector_total(lista_procesos); i++) {
                pid_t proceso_hijo = *(pid_t *) vector_get(lista_procesos, i);
                waitpid(proceso_hijo, NULL, 0);
            }
            printf("\nMr. Meeseeks (%d, %d): Adios, un placer ayudarte!", 
                            getpid(), getppid());
        }
        kill(*pidPrimerMrM, SIGKILL); // matar al primer Mr M que inició todo
    }
    else { // proceso original
        // controlar el tiempo del caos planetario
        clock_t inicioRelojTotal = clock();
        double tiempoTotalInvertido = 0.0;

        while (1) {
            if (tiempoTotalInvertido > TIEMPOMAX) { // declarar caos planetario
                mensajeBomba();
                printf("\n* Box Mr.Meeseeks: Se ha decretado Caos Planetario! *");
                kill(*pidPrimerMrM, SIGKILL); // matar al primer Mr M que inició todo
                break;
            }
            else {
                if (*solucionado) {
                    int leerMensaje;
                    close(fd[1]); // solo lectura, cierra el descriptor de escritura
                    read(fd[0], &leerMensaje, sizeof(leerMensaje));
                    printf("-> Box informa: Mr. Meeseeks (%d) ha terminado la solicitud!", leerMensaje);
                    close(fd[0]); // cerrar el descriptor de lectura
                    break;
                }
                // estar escuchando con un pipe la variable "solucionado" y si se cumple 
                // recibir el pid del Mr M que completó la tarea 
            }
            tiempoTotalInvertido = (double)(clock() - inicioRelojTotal) / CLOCKS_PER_SEC;
        }

        wait(NULL); // esperar a que el 1er Mr Meeseeks resuelva la tarea
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
        printf("Esperando una solicitud: (%d) ", getpid());

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

            pid_t pid = crearFork(strcat(hileras, " -> [Cálculo matemático]" ), 1);

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

            pid_t pid = crearFork(strcat(path, " -> [Ejecutar un programa]"), 1);

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
            printf("\n¿Desea cambiar el tiempo? [y/n]\n>>> ");
            char respuesta;
            scanf(" %c", &respuesta);

            if (respuesta == 'y') {
                printf("\nIngrese el valor (en segundos):\n>>> ");
                scanf("%f", &TIEMPOMAX);
            }
            printf("\nTiempo establecido: %lf \n", TIEMPOMAX);
        }
        else if (solicitud == 5) {
            printf("\nArroz con pollo, falta esto!");
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
    //printf("%d", determinarHijos(100));
}