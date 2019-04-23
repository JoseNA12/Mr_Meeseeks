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
#include "Tarea.c"
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
static datos_compartidos* datos = NULL; // variables para el mutex lock
static vector *lista_procesos;

static int* fd; // Pipe Global

// Variables para la Bitácora
struct tarea vectorTareas[256]; // Vector de tareas
struct tarea tareaSolicitada; // Tarea individual
int indiceTareas; // Indice para el vector tareas. Se aumenta por cada estructura introducido


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

float determinarDificultad(float max, float min){
    float dificultad = distribucionNormal() * getNumDistrNormal(max, min);
    if(dificultad > 100){
        dificultad = 100;
    }
    return dificultad;
}

// Determinar la cantidad de Mr Meeseeks a crear
int determinarHijos(float dificultad) {
    if (dificultad >= 85.01) { // 100 - 85.01 = 0 hijos
        return 0;
    } 
    else if (85 >= dificultad && dificultad > 45){ // 85 - 45.01 = min 1 hijo
        return getNumDistrNormal(45, 1);
    } else { // 45 - 0 = min 3 hijos
        return getNumDistrNormal(85, 3);
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
    int cantidadMrM; // Cantidad de MrM utilizados durante la ejecución
    pid_t procesoBoxMrMeeseek = getpid(); // proceso original (sin haber creado ningun mr meeseek)

    // Datos de memoria compartida
    solucionado = mmap(NULL, sizeof *solucionado, PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0); // Crear la variable compartida
	*solucionado = 0; // 0 -> sin solucion, 1 -> solucionado
    
    lista_procesos = mmap(NULL, sizeof *lista_procesos, PROT_READ | PROT_WRITE, 
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0); // Crear la variable compartida

    vector_init(lista_procesos);     

    initDatosCompartidos(); // inicializar memoria compartida

    // ----------- pipes -------------
    char *estadoCompletado = malloc(sizeof(int));;
    char bufferEstado[20];
    int readpipe[2];
    int writepipe[2];
    int a = pipe(readpipe);
    int b = pipe(writepipe);
    // --------------------------------

    // Consultas al usuario
    printf("\nEscribe tu petición:\n>>> ");
    char caracter = '0';
    short i = 0;
    setbuf(stdin, NULL);
    while(caracter != '\n') {    // termina de leer la entrada de datos
        caracter = getchar();
        if (caracter == '\n') { break; }
        peticion[i] = caracter;        
        i++;
    }setbuf(stdin, NULL);
    peticion[i] = '\0';

    strcpy(tareaSolicitada.peticion,peticion); // Copiar peticion en la estructura de bitácora

    printf("\n¿Conocés la dificultad de tu petición? [y/n]:\n>>> ");
    scanf(" %c", &respuesta);

    if (respuesta == 'y') {
        printf("\nRango de 0 a 100: >>> ");
        scanf("%f", &dificultad);
    }
    else {
        dificultad = distribucionNormal() * getNumDistrNormal(DIFICULTADMAX, DIFICULTADMIN);
        if(dificultad > 100){
            dificultad = 100;
        }
    }

    // Crear el primer Mr Meeseek
    pid_t primerMrMeekseek = crearFork(peticion, 1);
    pid_t *mrMeeseekAyudante;

    if(primerMrMeekseek == 0){
        vector_free(lista_procesos); // limpiar la lista
        vector_init(lista_procesos); // inicializa la lista

        while(!*solucionado){
            printf("\nMr. Meeseeks (%d, %d): Dificultad de %f%%", getpid(), getppid(), dificultad);
            printf("\nMr. Meeseeks (%d, %d): ", getpid(), getppid()); printf("%s", mensajeEspera()); 
            tiempo_solicitud = setTiempoSolicitud(dificultad);
            sleep(tiempo_solicitud);

            if (dificultad > 85.00) {
                pthread_mutex_lock(&datos->mutex); // bloquear el recurso compartido 
                if (!*solucionado) {
                    // ----------- pipes -------------
                    close(readpipe[0]);
                    sprintf(estadoCompletado, "%d", getpid());
                    write(writepipe[1], estadoCompletado, strlen(estadoCompletado)+1); 
                    close(writepipe[1]);
                    free(estadoCompletado);
                    // --------------------------------
                    //printf("\n ======================= Mr. Meeseeks (%d, %d): Tarea Solucionada! =====================",getpid(), getppid());
                    //printf("\n");
                    *solucionado = 1; 
                }
                pthread_mutex_unlock(&datos->mutex); // liberar el recurso compartido
            }
            else{
                if(!*solucionado){
                    int ayudantesMrM = determinarHijos(dificultad); // Determinar si el Mr M necesita ayuda
                    printf("\nMr. Meeseeks (%d, %d): Necesitaré ayuda!. Me multiplicaré %d veces",getpid(), getppid(), ayudantesMrM);
                    
                    if(dificultad != 0.0){ // disminuir la dificultad para los hijos
                        dificultad = getNumDistrNormal(DIFICULTADMAX, dificultad);
                    }  
                    N++;
                    for (int I = 1; I <= ayudantesMrM; I++) {
                        printf("\n ");
                        if(!*solucionado){
                            mrMeeseekAyudante = (pid_t*)malloc(sizeof(pid_t));
                            *mrMeeseekAyudante = crearFork(peticion, I);

                            if(*mrMeeseekAyudante == 0){
                                break;
                            }else if(*mrMeeseekAyudante < 0){
                                printf("\n  Ha ocurrido un ERROR al crear el nuevo Mr.Meeseek");
                                break;
                            }else{
                                pthread_mutex_lock(&datos->mutex); // bloquear el recurso compartido 
                                //printf("\nMr. Meeseeks (%d, %d): Estoy agregando mi ayudante %d a la lista",getpid(), getppid(),*mrMeeseekAyudante);
                                vector_add(lista_procesos, mrMeeseekAyudante);
                                pthread_mutex_unlock(&datos->mutex); // liberar el recurso compartido
                            }
                        }else{ // Se encontro solucion
                            break; 
                        }                   
                    } 
                }else{ // Se encontro solucion
                    break;
                }    
            }
        }
        if(*mrMeeseekAyudante != 0){
            for (int i = 0; i < vector_total(lista_procesos); i++) {
                pid_t proceso_hijo = *(pid_t *) vector_get(lista_procesos, i);
                printf("\nMr. Meeseeks (%d, %d): Adios, un placer ayudarte!", proceso_hijo, getpid());
                printf("\nMr. Meeseeks (%d, %d): Destruyendo al proceso %d!", getpid(), getppid(),proceso_hijo); 
                free((pid_t *) vector_get(lista_procesos, i));
                kill(proceso_hijo, SIGKILL);
                cantidadMrM++;
            }
        }else{ // Cuando hijo termina antes que el padre. Ponerlo a dormir para que el padre lo mate luego
            sleep(300);
        }
    }else{  // Box MrM
        // controlar el tiempo del caos planetario
        clock_t inicioRelojTotal = clock();
        double tiempoTotalInvertido = 0.0;

        while (1) {
            if (tiempoTotalInvertido > TIEMPOMAX) { // declarar caos planetario
                mensajeBomba();
                printf("\n* Box Mr.Meeseeks: Se ha decretado Caos Planetario! *");
                kill(primerMrMeekseek, SIGKILL); // matar al primer Mr M que inició todo
                break;
            }
            else {
                if (*solucionado) {
                    // ----------- pipes -------------
                    close(readpipe[1]);
                    read(writepipe[0], bufferEstado, sizeof(bufferEstado));
                    close(writepipe[0]);
                    printf("\n* Box Mr.Meeseeks: Se ha resuelto la solicitud -> Mr Meeseeks (%s)", bufferEstado);
                    // -------------------------------
                    //tareaSolicitada.estado = 1;
                    break;
                }
                // estar escuchando con un pipe la variable "solucionado" y si se cumple 
                // recibir el pid del Mr M que completó la tarea 
            }
            tiempoTotalInvertido = (double)(clock() - inicioRelojTotal) / CLOCKS_PER_SEC;
        }
        tareaSolicitada.cantidadMrM = cantidadMrM;
        tareaSolicitada.tiempoDuracion = tiempoTotalInvertido;
        vectorTareas[indiceTareas] = tareaSolicitada;
        wait(NULL); // esperar a que el 1er Mr Meeseeks resuelva la tarea
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
    indiceTareas = 0;
    memset(vectorTareas, 0, sizeof(vectorTareas));
    while(1) {
        printf("\n\n======================== Box Mr.Meeseeks ========================\n");
        printf("        [1] - Consulta textual\n");
        printf("        [2] - Cálculo matemático\n");
        printf("        [3] - Ejecutar un programa\n\n");
        printf("        [4] - Tiempo máximo para el caos planetario\n");
        printf("=================================================================\n\n");
        printf("(%d) Esperando una solicitud: ", getpid());

        int solicitud;
        scanf("%d", &solicitud);
    
        if (solicitud == 1) { // Consulta textual
            consultaTextual();
        }
        else if (solicitud == 2) // Cálculo matemático
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

            pid_t pid = crearFork(strcat(hileras, " -> [Cálculo matemático]" ), 1);

            if (pid == 0) {
                char *sresultado = calculoMatematico(hileras);

                if (sresultado != NULL) {
                    printf("\nMr. Meeseeks (%d, %d): El resultado es: %s\n", getpid(), getppid(), sresultado);
                    estadoCompletado = "Completado";
                }
                else { 
                    printf("Mr. Meeseeks (%d, %d): Error en la operación ingresada!\n", getpid(), getppid());
                    estadoCompletado = "Incompleto";
                }

                printf("Mr. Meeseeks (%d, %d): Fue un placer servirle. Adios!", getpid(), getppid());

                free(sresultado);
                // ----------- pipes -------------
                close(readpipe[0]);
                write(writepipe[1], estadoCompletado, strlen(estadoCompletado)+1); 
                close(writepipe[1]);
                // --------------------------------
                kill(getpid(), SIGKILL);
            }
            else {
                wait(NULL); // Esperar que el proceso hijo complete su tarea
           
                // ----------- pipes -------------
                close(readpipe[1]);
                read(writepipe[0], bufferEstado, sizeof(bufferEstado));
                close(writepipe[0]);
                // -------------------------------

                tiempoTotalInvertido = (double)(clock() - inicioRelojTotal) / CLOCKS_PER_SEC;
                tareaSolicitada = (struct tarea){.cantidadMrM = 1, .tiempoDuracion = tiempoTotalInvertido};
                strcpy(tareaSolicitada.peticion, hileras);
                strcpy(tareaSolicitada.estado, bufferEstado);
                vectorTareas[indiceTareas] = tareaSolicitada;
                indiceTareas++;
            }
        }
        else if (solicitud == 3) // Ejecutar un programa
        {
            char path[SIZE];
            // ----------- pipes -------------
            char *estadoCompletado;
            char bufferEstado[20];
            int readpipe[2];
            int writepipe[2];
            int a = pipe(readpipe);
            int b = pipe(writepipe);
            // --------------------------------

            clock_t inicioRelojTotal = clock();
            double tiempoTotalInvertido = 0.0;

            printf("\nIngrese el path/comando del programa:\n>>> ");
            scanf("%s", path);

            pid_t pid = crearFork(strcat(path, " -> [Ejecutar un programa]"), 1);

            if (pid == 0) {
                int status = ejecutarPrograma(path);

                if (status == 0) {
                    estadoCompletado = "Completado";
                    printf("\nMr. Meeseeks (%d, %d): El programa se ha ejecutado!", getpid(), getppid());
                }
                else {
                    estadoCompletado = "Incompleto";
                    printf("\nMr. Meeseeks (%d, %d): Error al ejecutar el programa ingresado!", getpid(), getppid());
                }
                printf("\nMr. Meeseeks (%d, %d): Fue un placer servirle. Adios!", getpid(), getppid());

                // ----------- pipes -------------
                close(readpipe[0]);
                write(writepipe[1], estadoCompletado, strlen(estadoCompletado)+1); 
                close(writepipe[1]);
                // -------------------------------

                kill(getpid(), SIGKILL);
            }
            else {
                wait(NULL);

                // ----------- pipes -------------
                close(readpipe[1]);
                read(writepipe[0], bufferEstado, sizeof(bufferEstado));
                close(writepipe[0]);
                // -------------------------------

                tiempoTotalInvertido = (double)(clock() - inicioRelojTotal) / CLOCKS_PER_SEC;
                tareaSolicitada = (struct tarea){.cantidadMrM = 1, .tiempoDuracion = tiempoTotalInvertido};
                strcpy(tareaSolicitada.peticion, path); // peticion
                strcpy(tareaSolicitada.estado, bufferEstado); // mensaje via pipe proveniente del hijo
                vectorTareas[indiceTareas] = tareaSolicitada;
                indiceTareas++;
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
        else if (solicitud == -1)
        {
            printf("\n\n==================================== Bitacora ====================================");
            for (int i = 0; i < indiceTareas; i++) {
                
                printf("\n  Tarea solicitada: %s",vectorTareas[i].peticion);
                printf("\n  Cantidad MrM empleados: %d",vectorTareas[i].cantidadMrM);
                printf("\n  Duración: %f",vectorTareas[i].tiempoDuracion);
                printf("\n  Estado de tarea: "); printf("%s", vectorTareas[i].estado);
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
    box_Mr_Meeseeks();
}