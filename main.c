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
int N = 1;
int I = 1;

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
    float tiempo = (100 - dificultad) / 100 * 5;
    if (0.5 > tiempo){
        tiempo = 0.5;
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

pid_t crearFork() {
    pid_t pid = fork();
    printf("\nHi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n\n", getpid(), getppid(), N, I);

    return pid;
}

void consultaTextual() {

    char peticion[SIZE];
    char respuesta;
    float dificultad;
    float tiempo_tarea;

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

    pid_t meeseeksPadre = crearFork();

    tiempoActual = time(NULL);
    structActual = *((struct tm*)localtime(&tiempoActual));

    while (difftime(tiempoActual, tiempoInicio) < TIEMPOMAX) {

    }
    
}

int calculoMatematico() {
    char hileras[SIZE];
    printf("\nIngrese la hilera matemática:\n>>> ");
    scanf("%s", hileras);

    int a,b;
    char op;
    int resultado; 
    if (sscanf(hileras, "%d %c %d", &a, &op, &b) != 3) { // parsear el string y obtener los valores
        printf("\n*** Formato inválido de las hileras ***");
        return resultado;
    }
    else {
        switch (op) {
            case '+': resultado = a + b; break;
            case '-': resultado = a - b; break;
            case '*': resultado = a * b; break;
            case '/': resultado = a / b; break;
            default:
                printf("\n*** Operador de las hileras inválido ***\n");
        }
        return resultado;
    }
}

int ejecutarPrograma() {
    char path[SIZE];
    printf("\nIngrese el path/comando del programa:\n>>> ");
    scanf("%s", path);

    return system(path); // valor de retorno del programa ejecutado (0 o 1)
}

void box_Mr_Meeseeks() {
    while(1) {
        printf("\n\n===================== Box Mr.Meeseeks =====================\n\n");
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
            pid_t pid = fork();

            if (pid == 0) {
                printf("\nHi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n", getpid(), getppid(), 1, 1);
                int resultado = calculoMatematico();
                printf("\nMr. Meeseeks (%d, %d, %d, %d): El resultado es: %d\n", getpid(), getppid(), 1, 1, resultado);
                kill(getpid(), SIGTERM); // Eliminar el proceso hijo
            }
            else {
                wait(NULL); // Esperar que el proceso hijo complete su tarea
            }
        }
        else if (solicitud == 3) // Ejecutar un programa
        {
            pid_t pid = fork();

            if (pid == 0) {
                printf("\nHi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n", getpid(), getppid(), 1, 1);
                
                int status = ejecutarPrograma();

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