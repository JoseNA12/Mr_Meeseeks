#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#define SIZE 256
int instancia = 1;
int nivel = 1;

// [3]: google-chrome, geany, atom, pinta


int generarDificultad(){
    //Obtener una semilla para la generación de randoms
    srand(time(NULL));
    /*
     * Para generar un número aleatorio que se aproxima a una distribución normal
     * se generan 10 números random desde la función rand() y luego se dividen entre 10
    */
    int randomsSum = 0;
    for (int c = 1; c <= 10; c++) {
        randomsSum += rand() % 101;
    }
    return randomsSum/10;
}

void consultaTextual() {
    char peticion[SIZE];
    char respuesta;
    int dificultad = -1;

    pid_t pid = fork();

    if (pid == 0) {
        printf("\nHi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n\n", getpid(), getppid(), nivel, instancia);
        printf("Escribe tu petición:\n>>> ");
        scanf("%s", peticion);
        printf("\n¿Conocés la dificultad de tu petición? [y/n]:\n>>> ");
        scanf("%c", &respuesta);

        if (respuesta == 'y') {
            printf("\nRango de 0 a 100: >>> ");
            scanf("%d", &dificultad);
        }
        else {
            dificultad = generarDificultad();
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
                printf("\nHi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n", getpid(), getppid(), nivel, instancia);
                int resultado = calculoMatematico();
                printf("\nMr. Meeseeks (%d, %d, %d, %d): El resultado es: %d\n", getpid(), getppid(), nivel, instancia, resultado);
                
            }
            else {
                wait(NULL); // Esperar por el proceso hijo creado
            }
        }
        else if (solicitud == 3) // Ejecutar un programa
        {
            pid_t pid = fork();

            if (pid == 0) {
                printf("\nHi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)\n", getpid(), getppid(), nivel, instancia);
                
                int status = ejecutarPrograma();

                switch (status) {
                    case -1:
                        printf("\nMr. Meeseeks (%d, %d, %d, %d): Error al ejecutar el programa ingresado!\n", getpid(), getppid(), nivel, instancia);
                        break;
                    
                    case 0:
                        printf("\nMr. Meeseeks (%d, %d, %d, %d): El programa se ha ejecutado!\n", getpid(), getppid(), nivel, instancia);
                        break;
                        
                    default:
                        break;
                }
            }
            else {
                wait(NULL); // Esperar por el proceso hijo creado
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