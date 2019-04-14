#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <math.h>

#define SIZE 256 // Tamaño de la variable para recibir el input del usuario
#define TIEMPOMAX 90 // Tiempo máximo antes de que todos los Mr. Meeseeks entren en un caos planetario
#define TIEMPOMAXREQ 5 // Tiempo máximo en responder una petición
#define TIEMPOMINREQ 0.5 // Tiempo mínimo en responder una petición
// Parametros de la distribucion normal
#define MEDIA 0
#define VARIANZA 1
#define RANGO 1
#define N 10000

#define DIFICULTADMAX 100
#define DIFICULTADMIN 0

// [3]: google-chrome, geany, atom, pinta
// gcc main.c -o main -lm


// http://cypascal.blogspot.com/2016/02/crear-una-distribucion-normal-en-c.html
float distribucionNormal(){
    srand(time(NULL));  // Reiniciar la semilla de rand()
    int i = 1; float aux;

    for(i; i <= N; i++){
        aux += (float)rand()/RAND_MAX;
    }
    return fabs(VARIANZA * sqrt((float)12/N) * (aux - (float)N/2) + MEDIA) * RANGO;
}

// Algoritmo Marsaglia
float getNumDistrNormal(){
    float value, x, y, rsq, f;

    do {
        x = 2.0 * rand() / (float)RAND_MAX - 1.0;
        y = 2.0 * rand() / (float)RAND_MAX - 1.0;
        rsq = x * x + y * y;
    } while( rsq >= 1. || rsq == 0. );
    
    f = sqrt( -2.0 * log(rsq) / rsq );
    // (x * f) is a number between [-3, 2.9]
    // (x * f) + 3 to get a number between [0, 5.9]
    // ((x * f) + 3) * 100 / 5.9 to get a number between [0, 100]
    value = ((x * f) + 3) * DIFICULTADMAX / 5.9; // maxNum = highest number
    if (value > DIFICULTADMAX) value = DIFICULTADMAX; // In case value is > 100
    else if (value < DIFICULTADMIN) value = DIFICULTADMIN; // In case value is < 0

    return value;
}

void consultaTextual() {
    char peticion[SIZE];
    char respuesta;
    float dificultad;

    pid_t pid = fork();

    if (pid == 0) {
        printf("\nHi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n\n", getpid(), getppid(), 1, 1);
        printf("Escribe tu petición:\n>>> ");
        scanf("%s", peticion);
        printf("\n¿Conocés la dificultad de tu petición? [y/n]:\n>>> ");
        scanf(" %c", &respuesta);

        if (respuesta == 'y') {
            printf("\nRango de 0 a 100: >>> ");
            scanf("%f", &dificultad);
        }
        else {
            dificultad = distribucionNormal() * getNumDistrNormal();
        }
    }
    else {
        wait(NULL); // Esperar por el proceso hijo creado
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