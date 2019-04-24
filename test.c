#include <stdio.h>          /* printf()                 */
#include <stdlib.h>         /* exit(), malloc(), free() */
#include <sys/types.h>      /* key_t, sem_t, pid_t      */
#include <sys/shm.h>        /* shmat(), IPC_RMID        */
#include <errno.h>          /* errno, ECHILD            */
#include <semaphore.h>      /* sem_open(), sem_destroy(), sem_wait().. */
#include <fcntl.h>          /* O_CREAT, O_EXEC          */
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <math.h>


void semaforo() {
	int i;                        /*      loop variables          */
    key_t shMemoryKey;            /*      shared memory key       */
    int shMemoryID;               /*      shared memory id        */
    sem_t *miSemaforo;            /*      synch semaphore         *//*shared */
    pid_t pid;                    /*      fork pid                */
    int *sharedVariable;          /*      shared variable         *//*shared */
    unsigned int n;               /*      fork count              */
    unsigned int value;           /*      semaphore value         */

    /* initialize a shared variable in shared memory */
    shMemoryKey = ftok("/dev/null", 5);       /* valid directory name and a number */
    printf("shMemoryKey for sharedVariable = %d\n", shMemoryKey);
    shMemoryID = shmget(shMemoryKey, sizeof (int), 0644 | IPC_CREAT);
    if (shMemoryID < 0){                           /* shared memory error check */
        perror("shmget\n");
        exit(1);
    }

    sharedVariable = (int *) shmat (shMemoryID, NULL, 0);   /* attach sharedVariable to shared memory */
    *sharedVariable = 0;
    printf("sharedVariable=%d is allocated in shared memory.\n\n", *sharedVariable);

    /********************************************************/

    printf("How many children do you want to fork?\n");
    printf("Fork count: ");
    scanf("%u", &n);

    printf("What do you want the semaphore value to be?\n");
    printf("Semaphore value: ");
    scanf("%u", &value);

    /* initialize semaphores for shared processes */
    miSemaforo = sem_open("pSem", O_CREAT | O_EXCL, 0644, value); 
    /* name of semaphore is "pSem", semaphore is reached using this name */

    printf("semaphores initialized.\n\n");


    /* fork child processes */
    for (i = 0; i < n; i++){
        pid = fork();
        if (pid < 0) {
        /* check for error      */
            sem_unlink("pSem");   
            sem_close(miSemaforo);  
            /* unlink prevents the semaphore existing forever */
            /* if a crash occurs during the execution         */
            printf ("Fork error.\n");
        }
        else if (pid == 0)
            break;                  /* child processes */
    }


    /******************************************************/
    /******************   PARENT PROCESS   ****************/
    /******************************************************/
    if (pid != 0) {
        /* wait for all children to exit */
        while (pid = waitpid (-1, NULL, 0)){
            if (errno == ECHILD)
                break;
        }

        printf ("\nParent: All children have exited.\n");

        /* shared memory detach */
        shmdt(sharedVariable);
        shmctl(shMemoryID, IPC_RMID, 0);

        /* cleanup semaphores */
        sem_unlink("pSem");   
        sem_close(miSemaforo);  
        /* unlink prevents the semaphore existing forever */
        /* if a crash occurs during the execution         */
        exit(0);
    }

    /******************************************************/
    /******************   CHILD PROCESS   *****************/
    /******************************************************/
    else {
        sem_wait(miSemaforo);           /* sharedVariable operation */
        printf("  Child(%d) is in critical section.\n", i);
        sleep(1);
        *sharedVariable += i % 3;              /* increment *sharedVariable by 0, 1 or 2 based on i */
        printf("  Child(%d) new value of *sharedVariable=%d.\n", i, *sharedVariable);
        sem_post(miSemaforo);           /* V operation */
        exit(0);
    }
}

void tiempo() {
    clock_t inicioRelojTotal;
    double tiempoTotalInvertido = 0.0;

    float TIEMPOMAX = 4;

    while (tiempoTotalInvertido < TIEMPOMAX) {
        printf(" tiempo: %lf\n", tiempoTotalInvertido);
        tiempoTotalInvertido = (double)(clock() - inicioRelojTotal) / CLOCKS_PER_SEC;
    }

    /*clock_t inicioRelojTotal = clock(); 

    double tiempoTotalInvertido = 0.0;
    double tiempoPensarInvertido = 0.0;
    float TIEMPOMAX = 3.0;
    float tiempoPensar = 4.1;

    while (tiempoTotalInvertido < TIEMPOMAX) {
        clock_t inicioRelojPensar = clock();
        tiempoPensarInvertido = 0.0;

        while (tiempoPensarInvertido < tiempoPensar) {
            clock_t temp_1 = clock();
            tiempoPensarInvertido = (double) (temp_1 - inicioRelojPensar) / CLOCKS_PER_SEC;
            printf("Pensar: %lf\n", tiempoPensarInvertido);
        }
        clock_t temp_2 = clock();
        tiempoTotalInvertido = (double) (temp_2 - inicioRelojTotal) / CLOCKS_PER_SEC;
        printf("--------------Total: %lf\n", tiempoTotalInvertido);
        sleep(1);
    }*/
}

void pruebaFork() {
    printf("Primera vez\n");
    pid_t padre;  

    for (int i = 0; i < 5; i++) {
        if (padre == 1) {
            padre = fork(); 
            
            if (padre == 0) { sleep(10000); }
        }
        printf("forki\n");
    }
}

// https://stackoverflow.com/questions/22502124/communication-between-child-and-parent-processes-in-c-linux-parent-process-not
void pruebaPipe() {

    pid_t pid;
    /* Hope this is big enough. */
    char buf[1024]; char buf2[1024];
    char *cp = "0"; char *cp2 = "0";

    int readpipe[2];
    int writepipe[2];
    int a = pipe(readpipe);
    int b = pipe(writepipe);
    // check a and b

    pid=fork();
    // check pid

    if(pid==0)
    { //CHILD PROCESS
        /*close(readpipe[1]);
        close(writepipe[0]);
        read(readpipe[0],buf,sizeof(buf));
        printf("\nREAD = %s",buf);*/
        close(readpipe[0]);
        cp="2";
        write(writepipe[1],cp,strlen(cp)+1);
        close(writepipe[1]);
    }
    else
    { //PARENT PROCESS
        /*close(readpipe[0]);
        close(writepipe[1]);
        cp="HI!! YOU THERE";
        write(readpipe[1],cp,strlen(cp)+1);*/
        close(readpipe[1]);
        read(writepipe[0],buf,sizeof(buf));
        printf("\n1er RECEIVED %s\n",buf);
        close(writepipe[0]);
    }

}
  
int main (int argc, char **argv){
	//semaforo();

    //tiempo();

    //pruebaFork();

    //pruebaPipe();
    char *estadoCompletado = malloc(sizeof(int));
    char *p1 = malloc(sizeof(int)); char *p2 = malloc(sizeof(int));
    char *p3 = malloc(sizeof(int)); p3 = "Solicitud completada! :)";
    sprintf(p1, "%d", getpid());
    sprintf(p2, "%d", getppid());
    snprintf(estadoCompletado, 120, "(%s, %s): %s", p1, p2, p3);

    printf("%s", estadoCompletado);

    int cantidad_procesos;
	
	printf("\nIngrese la cantidad de procesos ha crear: \n");
	scanf("%d", &cantidad_procesos);

    pid_t primerHijueputa = fork();
	pid_t pid = 1;

    if (!primerHijueputa) {
    
        clock_t inicioRelojTotal = clock();
        double tiempoTotalInvertido = 0.0;

        float TIEMPOMAX = 4;

        int i = 0;
        for (i = 0; i < cantidad_procesos; i++) {
            if (pid > 0) { // solo el padre cre hijos
                pid = fork();inicioRelojTotal = clock();
            }
            //else { break; }
        }
        
        if (pid < 0) { // ocurrió un error
            fprintf(stderr, "Fork fallo"); 
            return 1;
        }
        else if (pid == 0) { // soy el proceso hijo
            // execlp("/bin/ls", "ls", NULL);
            // sleep(3); // hacer el hijo huerfano
            tiempoTotalInvertido = (double)(clock() - inicioRelojTotal) / CLOCKS_PER_SEC;
            printf("\nHijo pid: %d, ppid: %d , %lf\n", getpid(), getppid(), tiempoTotalInvertido);
            
        }
        else { // soy el proceso padre
            //wait(NULL); // esperar por el ultimo creado
            while(wait(NULL) > 0); // -1 cuando no hay hijos
            printf("Padre pid: %d, ppid: %d\n", getpid(), getppid());
        
        }
    }
    else {
        wait(NULL);
        printf("\n* Box: terminé *\n");
    }

    return 0;
}