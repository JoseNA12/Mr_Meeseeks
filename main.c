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
#define TIEMPOMAX 300 // Tiempo máximo antes de que todos los Mr. Meeseeks entren en un caos planetario
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
// Compilar con: gcc main.c -o main -lm


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
float setTiempo(float dificultad) {
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
    } else {
        if (85 >= dificultad && dificultad > 45){ // 85 - 45.01 = min 1 hijo
            return getNumDistrNormal(45, 1);
        } else { // 45 - 0 = min 3 hijos
            return getNumDistrNormal(85, 3);
        }
    }
}

// https://bytefreaks.net/programming-2/c-programming-2/cc-pass-value-from-parent-to-child-after-fork-via-a-pipe
// Encargado de crear Pipes dentro de Padre e hijo
void comunicarProcesos(int fd[2], pid_t pid, char mensaje[SIZE]) {
    char *leerMensaje;

    if (pid != 0) { // padre
        close(fd[0]); // solo escritura

        // Enviar el mensaje en el descriptor de escritura.
        write(fd[1], &mensaje, sizeof(&mensaje));
        //printf("Padre (%d) envia envia: %s\n", getpid(), mensaje);

        close(fd[1]); // cierra el descriptor de escritura
    } 
    else {
        close(fd[1]); // solo lectura, cierra el descriptor de escritura

        // Ahora lee los datos (se bloqueará hasta que tenga éxito)
        read(fd[0], &leerMensaje, sizeof(leerMensaje));
        //printf("El hijo (%d) recibe: %s\n", getpid(), leerMensaje);

        close(fd[0]); // cerrar el descriptor de lectura
    }
}

int* crearPipe() { 
    static int fd[2]; 
    int retornoPipe;
    
    pipe(fd); // crear descriptores de tubería
    retornoPipe = pipe(fd);

    if (retornoPipe == -1) {
        printf("No se ha podido crear el pipe\n");
    }
    return fd;
}

pid_t crearFork(char peticion[SIZE]) {
    int* fd = crearPipe();
    pid_t pid = fork();

    comunicarProcesos(fd, pid, peticion);

    if (pid < 0) {
        fprintf(stderr, "Error al crear el Mr Meeseek :(\n");
    } 
    else if (pid == 0) { // Proceso hijo
        printf("\nHi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n", 
            getpid(), getppid(), N, I);
    }
    return pid;
}

void consultaTextual() {
    char peticion[SIZE], respuesta;
    float dificultad, tiempo_tarea;

    // variables del tiempo
    time_t tiempoInicio, tiempoActual;
    struct tm *structInicio;
    struct tm structActual;

    // Consultas al usuario
    printf("Escribe tu petición:\n>>> ");
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

    tiempo_tarea = setTiempo(dificultad);

    // Definir la hora de inicio de la tarea
    time(&tiempoInicio); // Segundos que han pasado desde January 1, 1970
    structInicio = localtime(&tiempoInicio); // Transforma esos segundos en la fecha y hora actual

    pid_t meeseekPadre = crearFork(peticion);

    /*tiempoActual = time(NULL);
    structActual = *((struct tm*)localtime(&tiempoActual));

    while (difftime(tiempoActual, tiempoInicio) < TIEMPOMAX) {
        printf("%f \n", difftime(tiempoActual, tiempoInicio));

        tiempoActual = time(NULL);
        structActual = *((struct tm*)localtime(&tiempoActual));
    }*/
    
}

// Calculo matematico de operaciones binarias
int calculoMatematico(char hileras[SIZE]) {
    int a,b;
    char op;
    int resultado; 
    if (sscanf(hileras, "%d %c %d", &a, &op, &b) != 3) { // parsear el string y obtener los valores
        printf("\nMr. Meeseeks (%d, %d, %d, %d): Formato inválido de las hileras\n", getpid(), getppid(), N+1, I+1);
        return resultado;
    }
    else {
        switch (op) {
            case '+': resultado = a + b; break;
            case '-': resultado = a - b; break;
            case '*': resultado = a * b; break;
            case '/': resultado = a / b; break;
            default:
                printf("\nMr. Meeseeks (%d, %d, %d, %d): Operador de las hileras inválido\n", getpid(), getppid(), N+1, I+1);
        }
        return resultado;
    }
}

int ejecutarPrograma(char path[SIZE]) {
    return system(path); // valor de retorno del programa ejecutado (0 o 1)
}

void box_Mr_Meeseeks() {
    while(1) {
        printf("\n\n======================== Box Mr.Meeseeks ========================\n\n");
        printf("Esperando una solicitud:\n");
        printf("    [1] - Consulta textual\n");
        printf("    [2] - Cálculo matemático\n");
        printf("    [3] - Ejecutar un programa\n\n>>> ");

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

                printf("\nMr. Meeseeks (%d, %d, %d, %d): El resultado es: %d\n", getpid(), getppid(), N+1, I+1, resultado);
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
                        printf("\nMr. Meeseeks (%d, %d, %d, %d): Error al ejecutar el programa ingresado!\n", getpid(), getppid(), 1, 1);
                        break;
                    
                    case 0:
                        printf("\nMr. Meeseeks (%d, %d, %d, %d): El programa se ha ejecutado!\n", getpid(), getppid(), 1, 1);
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