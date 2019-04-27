#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <math.h>
#include <semaphore.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#include "Vector.c"
#include "Solicitud.c"
#include "Proceso.c"
#include "MainForks_.h"
#include "Colores.c"

#define SIZE 256 // Capacidada de buffers

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

static datos_compartidos* datos = NULL; // variables para el mutex lock

static int* fd; // Pipe Global

// Variables para la Bitácora
struct vector *lista_solicitudes; // Vector de tareas

// Nivel de forks e instancias
int N = 1;

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

float randomFloat(float min, float max) {
	return (float) min + (rand()/(float)(RAND_MAX))*(max-min);
}

int randomInt(int min, int max) {
	return (rand() % (max-min)) + min;
}

// Determinar la cantidad de Mr Meeseeks a crear
int determinarHijos(float dificultad) {
    if (dificultad >= 85.01) { // 100 - 85.01 = 0 hijos
        return 0;
    } 
    else if (85 >= dificultad && dificultad > 45){ // 85 - 45.01 = min 1 hijo
        return randomInt(1, 10); // 45
    } else { // 45 - 0 = min 3 hijos
        return randomInt(3, 15); // 85
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
        printf("\nMr. Meeseeks (%d, %d): Solicitud recibida: '%s'", getpid(), getppid(), leerMensaje);

        close(fd[0]); // cerrar el descriptor de lectura
    }
}

int* crearPipe() {
    static int fd_temp[2]; 
    int retornoPipe;
    
    retornoPipe = pipe(fd_temp); // crear descriptores de tubería

    if (retornoPipe == -1) {
        printf("\nNo se ha podido crear el pipe!");
    }
    return fd_temp;
}

pid_t crearFork(char peticion[SIZE], int pI) {
    fd = crearPipe();
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "\nError al crear el Mr Meeseek :(\n");
    } 
    else if (pid == 0) { // Proceso hijo
        //pthread_mutex_lock(&datos->mutex); // bloquear el recurso compartido 
        Bold_Blue(); printf("\nHi I'm Mr Meeseeks! Look at Meeeee. (%d, %d, %d, %d)", 
            getpid(), getppid(), N, pI); Reset_Color();
        //pthread_mutex_unlock(&datos->mutex); // liberar el recurso compartido
    }
    // Comunicacion entre 2 procesos mediante un pipe
    comunicarProcesos(fd, pid, peticion);

    return pid;
}

// funcion para obtener mensajes random de espera en consola
char* mensajeEspera() {
    switch (randomInt(1, 6)) {
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

struct solicitud consultaTextual() {
    char peticion[SIZE], respuesta;
    float dificultad, tiempo_solicitud;
    static int *totalMrMeeseeks; // Cantidad de MrM utilizados durante la ejecución
    struct solicitud tareaSolicitada; // Tarea individual
    static int *solucionado; // variable que define la solucion de una tarea
    int instanciaActual = 1;
    int contadorAyudantes;

    // ---------------------- Datos de memoria compartida ---------------------- 
    solucionado = mmap(NULL, sizeof *solucionado, PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0); // Crear la variable compartida
	*solucionado = 0; // 0 -> sin solucion, 1 -> solucionado
    
    totalMrMeeseeks = mmap(NULL, sizeof *totalMrMeeseeks, PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *totalMrMeeseeks = 1;    
    // -------------------------------------------------------------------------
    initDatosCompartidos(); // inicializar memoria compartida

    // ---------------------- pipes ------------------------
    char *estadoCompletado = malloc(sizeof(int));;
    char bufferEstado[20];
    int readpipe[2];
    int writepipe[2];
    int a = pipe(readpipe);
    int b = pipe(writepipe);
    // -----------------------------------------------------

    // --------------------------- Consultas al usuario ---------------------------
    printf("\nEscribe tu petición:\n>>> ");
    char caracter = '0';
    short i = 0;
    setbuf(stdin, NULL);
    while(caracter != '\n') {    // termina de leer la entrada de datos
        caracter = getchar();
        if (caracter == '\n') { break; }
        peticion[i] = caracter;        
        i++;
    }
    peticion[i] = '\0';

    printf("\n¿Conocés la dificultad de tu petición? [y/n]:\n>>> ");
    scanf(" %c", &respuesta);

    if (respuesta == 'y') {
        printf("\nRango de 0 a 100: >>> ");
        scanf("%f", &dificultad);
    }
    else {
        dificultad = distribucionNormal() * randomInt(DIFICULTADMIN, DIFICULTADMAX);
        if(dificultad > 100){
            dificultad = 100;
        }
    }
    // ------------------------------------------------------------------------------

    pid_t mrMeeseekAyudante;
    pid_t primerMrMeekseek = crearFork(peticion, instanciaActual);

    if(!primerMrMeekseek) {
        while (!*solucionado) {
            printf("\nMr. Meeseeks (%d, %d): Dificultad de %f%%", getpid(), getppid(), dificultad);
            printf("\nMr. Meeseeks (%d, %d): %s", getpid(), getppid(), mensajeEspera());
            sleep(randomFloat(TIEMPOMINREQ, TIEMPOMAXREQ));

            if (dificultad > 85) {
                pthread_mutex_lock(&datos->mutex); // bloquear el recurso compartido 
                if (!*solucionado) {
                    // ----------- pipes -------------
                    printf("TRATANDO DE   SOLUCIONAR");
                    close(readpipe[0]);
                    sprintf(estadoCompletado, "%d", getpid());

                    // -- info --
                    strcat(estadoCompletado, ", ");
                    char id[100];
                    sprintf(id, "%d", getppid());

                    strcat(estadoCompletado, id);
                    // - - - - - - - - - - 
                    strcat(estadoCompletado, ", ");
                    char nivel[100];
                    sprintf(nivel, "%d", N);

                    strcat(estadoCompletado, nivel);
                    // - - - - - - - - - -
                    strcat(estadoCompletado, ", ");
                    char ins[100];

                    sprintf(ins, "%d", instanciaActual);
                    strcat(estadoCompletado, ins);
                    // -- info --

                    write(writepipe[1], estadoCompletado, strlen(estadoCompletado)+1); 
                    close(writepipe[1]);
                    free(estadoCompletado);
                    // -------------------------------
                    *solucionado = 1;
                    Bold_Magenta(); printf("\nMr. Meeseeks (%d, %d, %d, %d): He resuelto la solicitud :)", getpid(), getppid(), N, instanciaActual); Reset_Color();
                }
                pthread_mutex_unlock(&datos->mutex); // liberar el recurso compartido
            }

            else if (dificultad <= 0.0) {
                while (1) { // Pensamiento infinito hasta causar caos planetario
                    printf("\nMr. Meeseeks (%d, %d): ", getpid(), getppid()); printf("%s", mensajeEspera());
                    int temp = randomInt(TIEMPOMINREQ, TIEMPOMAXREQ);
                    sleep(temp);
                }
            }
            else { // ocupa ayuda
                int ayudantesMrM = determinarHijos(dificultad); // Determinar si el Mr M necesita ayuda
                printf("\nMr. Meeseeks (%d, %d): Necesitaré ayuda!. Me multiplicaré %d veces",
                        getpid(), getppid(), ayudantesMrM);

                // ------------------------ Dificultad ------------------------
                float temp1 = randomInt(1, (int) dificultad);
                float temp2 = (dificultad/1000);
                dificultad = dificultad + (temp1 * temp2);
                // ------------------------------------------------------------

                N++;
                contadorAyudantes = 0;
                for (int i = 0; i < ayudantesMrM && !*solucionado; i++) {
                    instanciaActual += 1;
                    mrMeeseekAyudante = crearFork(peticion, instanciaActual);
                    if (mrMeeseekAyudante < 0) { // ocurrió un error
                        fprintf(stderr, "Fork fallo"); 
                        exit(1);
                    }
                    else if (mrMeeseekAyudante == 0) { // soy el proceso hijo
                        break;
                    }
                    else {
                        contadorAyudantes += 1;
                    }
                }
                if (mrMeeseekAyudante < 0) {
                    fprintf(stderr, "\nUn Mr. Meeseeks salió deforme :("); 
                }
                else if (!mrMeeseekAyudante) {
                    // breteo
                }
                else { // padre
                    pthread_mutex_lock(&datos->mutex); // bloquear el recurso compartido 
                    *totalMrMeeseeks += contadorAyudantes; // el padre contabiliza los hijos creados
                    pthread_mutex_unlock(&datos->mutex); // liberar el recurso compartido
                    while(wait(NULL) > 0); // -1 cuando no hay hijos
                }
            }
        }
        while(wait(NULL) > 0);
        printf("\nMr. Meeseeks (%d, %d): Adios, fue un placer ayudar!\n", getpid(), getppid());
    } 
    else {  // Box MrM
        // controlar el tiempo del caos planetario
        clock_t inicioRelojTotal = clock();
        double tiempoTotalInvertido = 0.0;
        char *estadoCompletado;
        while (1) {
            if (tiempoTotalInvertido > TIEMPOMAX) { // declarar caos planetario
                Bold_Yellow(); mensajeBomba(); Bold_Red();
                printf("\n* Box Mr.Meeseeks: Se ha decretado Caos Planetario! *"); Reset_Color();
                kill(primerMrMeekseek, SIGKILL); // matar al primer Mr M que inició todo
                estadoCompletado = "Incompleta";
                break;
            }
            else {
                if (*solucionado) {
                    // ----------- pipes -------------
                    close(readpipe[1]);
                    read(writepipe[0], bufferEstado, sizeof(bufferEstado));
                    close(writepipe[0]);
                    printf("\n* Box Mr.Meeseeks: Se ha resuelto la solicitud -> Mr. Meeseeks (%s)", bufferEstado);
                    estadoCompletado = "Completada";
                    // -------------------------------
                    break;
                }
            }
            tiempoTotalInvertido = (double)(clock() - inicioRelojTotal) / CLOCKS_PER_SEC;
        }
 
         // Agregar tarea solicitada a la bitácora.
        tareaSolicitada = (struct solicitud){
            .cantidadMrM = *totalMrMeeseeks,
            .tiempoDuracion = tiempoTotalInvertido
        };
        strcpy(tareaSolicitada.estado, estadoCompletado);
        strcat(peticion, " -> [Consulta Textual]");
        strcpy(tareaSolicitada.peticion, peticion);
        // ----------------------------------------------------------------
        while(wait(NULL) > 0); // esperar a que el 1er Mr Meeseeks resuelva la tarea
        return tareaSolicitada;    
    }
}

// Calculo matematico de operaciones binarias
char* calculoMatematico(char hileras[SIZE]) {
    int a,b;
    char op;
    int resultado;
    char *sresultado = malloc(sizeof(int));
    if (sscanf(hileras, "%d %c %d", &a, &op, &b) != 3) { // parsear el string y obtener los valores
        printf("\nMr. Meeseeks (%d, %d): Formato inválido de las hileras\n", getpid(), getppid());
        return NULL;
    }
    else {
        switch (op) {
            case '+': resultado = a + b; break;
            case '-': resultado = a - b; break;
            case '*': resultado = a * b; break;
            case '/': resultado = a / b; break;
            default:
                printf("\nMr. Meeseeks (%d, %d): Operador de las hileras inválido\n", getpid(), getppid());
                return NULL;
        }
        sprintf(sresultado, "%d", resultado); // resultado valido
        return sresultado;
    }
}

int ejecutarPrograma(char path[SIZE]) {
    return system(path); // valor de retorno del programa ejecutado (0 o 1)
}

void box_Mr_Meeseeks() {
    // ------------------------ Inicializar lista de solicitudes ------------------------
  //  lista_solicitudes = mmap(NULL, sizeof *lista_solicitudes, PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0); // Crear la variable compartida*/
    lista_solicitudes = (struct vector*)malloc(sizeof(struct vector));
    vector_free(lista_solicitudes); // limpiar la lista
    vector_init(lista_solicitudes);
    // ----------------------------------------------------------------------------------

    while(1) { 
        struct solicitud *tareaSolicitada; // Tarea individual
        tareaSolicitada = (struct solicitud*)malloc(sizeof(struct solicitud));
        Bold_Blue(); printf("\n\n======================== Box Mr.Meeseeks ========================\n");
        printf("        [1] - Consulta textual\n");
        printf("        [2] - Cálculo matemático\n");
        printf("        [3] - Ejecutar un programa\n\n");
        printf("        [4] - Tiempo máximo para el caos planetario\n");
        printf("=================================================================\n\n"); Reset_Color();
        
        printf("(%d) Esperando una solicitud: ", getpid());

        int opcionMenu;
        scanf("%d", &opcionMenu);
    
        if (opcionMenu == 1) { // Consulta textual
            // ----------- pipes -------------
            int readpipe[2];
            int writepipe[2];
            int a = pipe(readpipe);
            int b = pipe(writepipe);
            // --------------------------------

            pid_t pid = fork();

            if (!pid) {
                int pid_ = getpid();
                struct solicitud solicitudTextual;
                solicitudTextual = consultaTextual();

                if (pid_ == getpid()) {
                    // ----------- pipes -------------
                    close(readpipe[0]);
                    write(writepipe[1], &solicitudTextual, sizeof(struct solicitud)); 
                    close(writepipe[1]);
                    // --------------------------------
                }
                sleep(1);
                break;
            }
            else {
                // ----------- pipes -------------
                close(readpipe[1]);
                read(writepipe[0], tareaSolicitada, sizeof(struct solicitud));
                close(writepipe[0]);
                // --------------------------------

                vector_add(lista_solicitudes, tareaSolicitada);

                wait(NULL);
            }
        }
        else if (opcionMenu == 2) // Cálculo matemático
        {
            char hileras[SIZE];
            clock_t inicioRelojTotal = clock();
            double tiempoTotalInvertido = 0.0;
            
            printf("\nIngrese la hilera matemática:\n>>> ");
            scanf("%s", hileras);

            // ----------- pipes -------------
            char *estadoCompletado;
            char bufferEstado[20];
            int readpipe[2];
            int writepipe[2];
            int a = pipe(readpipe);
            int b = pipe(writepipe);
            // --------------------------------

            char temp[SIZE];
            strcpy(temp, hileras);

            pid_t pid = crearFork(strcat(temp, " -> [Cálculo matemático]"), 1);

            if (pid == 0) {
                char *sresultado = calculoMatematico(hileras);

                if (sresultado != NULL) {
                    printf("\nMr. Meeseeks (%d, %d, %d, %d): El resultado es: %s\n", getpid(), getppid(), 1, 1, sresultado);
                    estadoCompletado = "Completada";
                }
                else { 
                    printf("Mr. Meeseeks (%d, %d, %d, %d): Error en la operación ingresada!\n", getpid(), getppid(), 1, 1);
                    estadoCompletado = "Incompleta";
                }

                free(sresultado);
                // ----------- pipes -------------
                close(readpipe[0]);
                write(writepipe[1], estadoCompletado, strlen(estadoCompletado)+1); 
                close(writepipe[1]);
                // --------------------------------

                printf("Mr. Meeseeks (%d, %d, %d, %d): Fue un placer servirle. Adios!", getpid(), getppid(), 1, 1);
                break;
            }
            else {
                // ----------- pipes -------------
                close(readpipe[1]);
                read(writepipe[0], bufferEstado, sizeof(bufferEstado));
                close(writepipe[0]);
                // -------------------------------

                wait(NULL); // Esperar que el proceso hijo complete su tarea

                tiempoTotalInvertido = (double)(clock() - inicioRelojTotal) / CLOCKS_PER_SEC;

                // Agregar tarea solicitada a la bitácora.
                *tareaSolicitada = (struct solicitud){
                    .cantidadMrM = 1, 
                    .tiempoDuracion = tiempoTotalInvertido
                    };

                strcpy(tareaSolicitada->peticion, temp);
                strcpy(tareaSolicitada->estado, bufferEstado);

                vector_add(lista_solicitudes, tareaSolicitada);
                // ----------------------------------------------------------------
            }
        }
        else if (opcionMenu == 3) // Ejecutar un programa
        {
            char path[SIZE];
            // ----------- pipes -------------
            char *estadoCompletado = "Incompleta";
            char bufferEstado[20];
            int readpipe[2];
            int writepipe[2];
            int a = pipe(readpipe);
            int b = pipe(writepipe);
            // --------------------------------

            clock_t inicioRelojTotal = clock();
            double tiempoTotalInvertido = 0.0;

            printf("\nIngrese un path o comando a ejecutar:\n>>> ");
            scanf("%s", path);

            char temp[SIZE];
            strcpy(temp, path);

            pid_t pid = crearFork(strcat(temp, " -> [Ejecutar un comando/programa]"), 1);

            if (pid == 0) {
                printf("\n");
                int status = ejecutarPrograma(path);

                if (status == 0) {
                    estadoCompletado = "Completada";
                    printf("\nMr. Meeseeks (%d, %d, %d, %d): El comando/programa se ha ejecutado!", getpid(), getppid(), 1, 1);
                }
                else {
                    printf("\nMr. Meeseeks (%d, %d, %d, %d): Error al ejecutar el comando/programa ingresado!", getpid(), getppid(), 1, 1);
                }

                // ----------- pipes -------------
                close(readpipe[0]);
                write(writepipe[1], estadoCompletado, strlen(estadoCompletado)+1); 
                close(writepipe[1]);
                // -------------------------------

                printf("\nMr. Meeseeks (%d, %d, %d, %d): Fue un placer servirle. Adios!", getpid(), getppid(), 1, 1);
                break;
            }
            else {
                // ----------- pipes -------------
                close(readpipe[1]);
                read(writepipe[0], bufferEstado, sizeof(bufferEstado));
                close(writepipe[0]);
                // -------------------------------

                wait(NULL);

                tiempoTotalInvertido = (double)(clock() - inicioRelojTotal) / CLOCKS_PER_SEC;

                // Agregar tarea solicitada a la bitácora.
                *tareaSolicitada = (struct solicitud){
                    .cantidadMrM = 1, 
                    .tiempoDuracion = tiempoTotalInvertido
                    };

                strcpy(tareaSolicitada->peticion, temp);
                strcpy(tareaSolicitada->estado, bufferEstado);

                vector_add(lista_solicitudes, tareaSolicitada);
            }
        }
        else if (opcionMenu == 4) { // Configurar tiempo máximo caos planetario
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
        else if (opcionMenu == -1)
        {
            printf("\n\n==================================== Bitacora ====================================");
            for (int i = 0; i < vector_total(lista_solicitudes); i++) {
                struct solicitud *solicitudSeleccionada = (struct solicitud*) vector_get(lista_solicitudes, i);
                Bold_Yellow();
                printf("\n  Tarea solicitada: %s",solicitudSeleccionada->peticion);
                printf("\n  Mr. Meeseeks empleados: %d",solicitudSeleccionada->cantidadMrM);
                printf("\n  Duración: %f",solicitudSeleccionada->tiempoDuracion);
                printf("\n  Estado: "); printf("%s", solicitudSeleccionada->estado);
                Reset_Color();
                printf("\n| ------------------------------------------------------------------------------ |");
            }
            
            printf("\n\n*** La Box Mr.Meeseeks ha sido destruida! ***\n");
            break;
        }
        else
        {
            printf("\n*** Opción no válida ***");
        }
    }
}

int main(){
    srand(time(NULL));
    box_Mr_Meeseeks();
}