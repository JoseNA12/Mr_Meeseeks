#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>

#define SIZE 50

//Prototipos de funciones
int generarDificultad();
int mrMeeseeks(int, int, int[2], sem_t*, int*);
int solucionarProblema(int, int, int);
float getTiempoSolucion(int, int, int);
void dificultadCero();
void print(char*, sem_t*, int*);
void *thread_function(void *arg);

volatile int stop=0;

void sigalrm_handler( int sig ) {
    stop = 1;
}

struct thread_info {
	int pipe[2];
	int n;
	int i;
	long unsigned int ptid;
    };


//variables globales
int* sharedStatus;
    key_t shmkey;                 //shared memory key
    int shmid;                    //shared memory id
    sem_t sem;                   //synch semaphore

int main() {
    struct timeval tv1, tv2;
    char peticion[SIZE];
    int res;
    int dificultad = -1;
    int fd[2];
    pthread_t a_thread;
    struct sigaction sact;

    sigemptyset(&sact.sa_mask);
    sact.sa_flags = 0;
    sact.sa_handler = sigalrm_handler;
    sigaction(SIGALRM, &sact, NULL);


    //Obtener una semilla para la generación de randoms
    srand(time(NULL));

    /* Inicializar semaforo para variable compartida status */
    shmkey = ftok("/dev/null", 5);
    shmid = shmget(shmkey, sizeof (int), 0644 | IPC_CREAT);
    if (shmid < 0){
        perror("shmget\n");
        exit (1);
    }
    sharedStatus = (int *) shmat(shmid, NULL, 0);
    *sharedStatus = 0;
    /********************************************************/

    /* initialize semaphores for shared processes */
    sem_init (&sem, 0, 1);
    /* name of semaphore is "pSem", semaphore is reached using this name */

    pipe2(fd, O_DIRECT);

    struct thread_info thread;
    thread.pipe[0] = fd[0];
    thread.pipe[1] = fd[1];
    thread.n = 1;
    thread.i = 1;
    thread.ptid = getpid();


    while(1) {
        printf("Escriba su petición para activar The Meeseeks Box\n");
        scanf("%[^\n]s", peticion);
        printf("Cual es la dificultad de la tarea?: ");
        scanf("%u", &dificultad);

        if(dificultad < 0) {
            dificultad = generarDificultad();
        }
        if(dificultad == 0){
            dificultadCero();
        }
        gettimeofday(&tv1, NULL);
	    sem_wait(&sem);
        write(fd[1], peticion, SIZE);
        write(fd[1], &dificultad, sizeof(dificultad));
	    sem_post(&sem);

        //crea el hilo y ejecuta thread_function
        res = pthread_create(&a_thread, NULL, thread_function, &thread);
        if(res != 0) {
            sem_unlink ("pSem");
            sem_close(&sem);
            printf("Error al crear un Meeseeks\n");
	    exit(EXIT_FAILURE);
        }

        res = pthread_join(a_thread,NULL);
        if (res != 0){
            printf("Error al finalizar el thread.\n");
            exit(EXIT_FAILURE);
        }
        /* Eliminar memoria compartida */
        shmdt (sharedStatus);
        shmctl (shmid, IPC_RMID, 0);
        /* Limpiar semaforos */
        sem_unlink ("pSem");
        sem_close(&sem);
        break;
    }
    printf("El programa terminó\n");
    gettimeofday(&tv2, NULL);
    printf("El programa terminó, tiempo de ejecución: %f\n", (double)(tv2.tv_usec - tv1.tv_usec)/1000000 + (double)(tv2.tv_sec - tv1.tv_sec));
    close(fd[0]);
    close(fd[1]);
    return 0;
}

void *thread_function(void *arg){
    int res;
    float tiempoSolucion;
    int mrMeeseeksColaboradores, colabTot;
    pthread_t tid = pthread_self();
    pid_t ppid = getppid();
    char peticion[SIZE];
    int dificultad;
    int status;
    int dificultadCopy;
    time_t endwait;
    time_t start;
    time_t seconds;

    //leer argumentos del thread
    struct thread_info* thread = (struct thread_info*)arg;

    sem_wait(&sem);
    read(thread -> pipe[0], peticion, SIZE);
    read(thread -> pipe[0], &dificultad, sizeof(dificultad));
    sem_post(&sem);

    dificultadCopy = dificultad;

    //Obtener Tiempos de Solución y Mr Meeseeks Ayudantes
    int tmp = thread -> n;
    int tmp2 = thread -> i;
    long unsigned int ptid = thread -> ptid;
    tiempoSolucion = getTiempoSolucion(tmp, tmp2, dificultad);
    colabTot = mrMeeseeksColaboradores = solucionarProblema(tmp, tmp2, dificultad);
    colabTot++;

    printf("{\"Hi I'm Mr. Meeseeks Look at Meeeeee!\": \" \", \"tid\" : %lu, \"ptid\" : %lu, \"N\" : %d, \"i\" : %d}\n"
           "{\"Caaaaaaaan Doooooo\" : \"%s\", \"tid\": %lu, \"ptid\": %lu}\n"
           "{\"Hi I'm Mr. Meeseeks Look at Meeeeee!\" : \" \", \"Esta es la dificultad de mi peticion\": %d, \"tid\": %lu, \"ptid\": %lu}\n"
           "{\"Hi I'm Mr. Meeseeks Look at Meeeeee!\": \" \", \"Este es mi Tiempo de Solución\": %f, \"tid\": %lu, \"ptid\": %lu}\n"
           "{\"Hi I'm Mr. Meeseeks Look at Meeeeee!\" : \" \", \"Posiblemente llame a\" : %d, \"tid\": %lu, \"ptid\": %lu}\n""\n"
          , tid, ptid, tmp, tmp2, peticion, tid, ptid, dificultad, tid, ptid, tiempoSolucion, tid, ptid, mrMeeseeksColaboradores, tid, ptid);
    
    pthread_t * a_thread = malloc(sizeof(pthread_t)*mrMeeseeksColaboradores);
    struct thread_info * threadHijo = malloc(sizeof(struct thread_info)*mrMeeseeksColaboradores);
	    
    trabajo: //Tag del goto para que el meeseek siga reintentando su tarea hasta que alguno de los meeseeks termine

    while(1) {
        sem_wait (&sem);
        status = *sharedStatus;
        sem_post (&sem);

        if(status) {
            finHermano:

            printf("{\"Hi I'm Mr. Meeseeks Look at Meeeeee!\" : \"Algun Mr Meeseeks terminó el trabajo, fue un placer servirle\", \"tid\": %lu, \"ptid\": %lu}\n", tid, ptid);
	    free(a_thread);
	    free(threadHijo);
	    pthread_exit("thanks");
        }
	    
        for(int i = 0; i < mrMeeseeksColaboradores; i++){
	            threadHijo[i].pipe[0] = thread -> pipe[0];
		    threadHijo[i].pipe[1] = thread -> pipe[1];            

		    dificultadCopy = dificultadCopy == 0 ? 0 : (dificultadCopy + (thread -> n+thread -> i)/thread -> n);
		    sem_wait(&sem);
		    write(thread -> pipe[1], peticion, SIZE);	
		    write(thread -> pipe[1], &dificultadCopy, sizeof(dificultadCopy));
	 	    sem_post(&sem);

		    threadHijo[i].n = thread -> n+1;
		    threadHijo[i].i = i+1;
		    threadHijo[i].ptid = tid;
	
		    //crea el hilo y este ejecuta thread_function
		    res = pthread_create(&a_thread[i], NULL, thread_function, &threadHijo[i]);
		    if(res != 0) {
		        printf("Error al crear un Meeseeks\n");
			exit(EXIT_FAILURE);
		    }
	    

	}
	for(int i = 0; i < mrMeeseeksColaboradores; i++){
	    res = pthread_join(a_thread[i],NULL);
	    if (res != 0){
	        printf("Error al finalizar el thread.\n");
	        exit(EXIT_FAILURE);
	    }
	}
            if (dificultad > generarDificultad()) {	
		sem_wait(&sem);
                if(*sharedStatus){
                    sem_post(&sem);
                    goto finHermano;
                }
                else {
                    *sharedStatus = 1;
                    sem_post(&sem);
		    start = time(NULL);
	            seconds = (unsigned int) tiempoSolucion+1;
	            endwait = start + seconds;


    		    while(start < endwait){
			sleep(1);
			start = time(NULL);
		    }             	
		       

                    printf("{\"Hi I'm Mr. Meeseeks Look at Meeeeee!\" : \"Terminé mi trabajo, fue un placer servirle\", \"tid\": %lu, \"ptid\": %lu}\n", tid, ptid);
		    free(a_thread);
		    free(threadHijo);
                    return 0;
                }
            }
            else {
                printf("{\"Hi I'm Mr. Meeseeks Look at Meeeeee!\" : \"Caaaaaaaan't doooooooooo, voy a reintentar mi trabajo\", \"tid\": %lu, \"ptid\": %lu}\n", tid, ptid);
                dificultad = dificultad == 0 ? 0 : dificultad + 1;
                goto trabajo;
            }
    }

}

int generarDificultad(){
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

int solucionarProblema(int n, int i, int dificultad){
    float probabilidadMrMeeseeks;
    int mrMeeseekscolaboradores = 0;


    if(dificultad == 0){
        return 10;
    }
    if(dificultad > 85){
        //La puede solucionar solo
        dificultad = 0;
    //100 -dificultad;
  //      probabilidadMrMeeseeks = ((float)dificultad/25)*100;
    }
    else if(dificultad > 45){
        //Tiene que llamar al menos 1 más
        dificultad = 85 -dificultad;
        probabilidadMrMeeseeks = ((float)dificultad/40)*100;
        mrMeeseekscolaboradores = 1;
    }
    else{
        //Tiene que llamar al menos 2 más
        dificultad = 45 -dificultad;
        probabilidadMrMeeseeks = ((float)dificultad/45)*100;
        mrMeeseekscolaboradores = 2;
    }
    //Se hace uso de la función generadora de dificultad para obtener un número random de 0 a 100
    probabilidadMrMeeseeks += (5*n + 2*i)/5;
    float rand = (float)generarDificultad();
    probabilidadMrMeeseeks -= rand;
    while(probabilidadMrMeeseeks > 0){
        mrMeeseekscolaboradores++;
        rand = (float)generarDificultad();
        probabilidadMrMeeseeks -= rand;
    }
    return mrMeeseekscolaboradores;
}

float getTiempoSolucion(int n, int i, int dificultad){
    float tiempo;
    if(dificultad == 0)
        return (float) 5.0;
    if(dificultad >= 85){
        tiempo = (5*(100-dificultad))/100;
        if(dificultad > 92)
            tiempo = (float) (5.0 - tiempo);
    }
    else if(dificultad > 45){
        tiempo = (5*(85-dificultad))/85;
        if(dificultad > 65)
            tiempo = (float) (5.0 - tiempo);
    }
    else{
        tiempo = (5*(45-dificultad))/45;
        if(dificultad > 22)
            tiempo = (float) (5.0 - tiempo);
    }
    tiempo += ((1/n)+(1/i))/2.0;
    return tiempo;
}

void dificultadCero(){
    printf("Oh no you have asked for a very hard task\n");
    printf("I hope you have a very powerfull computer...\n");
    alarm(300);//Esperar 5 minutos para matar al programa
}