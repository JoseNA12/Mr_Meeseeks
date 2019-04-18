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

int main (int argc, char **argv){
	//semaforo();

    tiempo();

    //pruebaFork();
}