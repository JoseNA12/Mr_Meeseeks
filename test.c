#include <stdio.h>          /* printf()                 */
#include <stdlib.h>         /* exit(), malloc(), free() */
#include <sys/types.h>      /* key_t, sem_t, pid_t      */
#include <sys/shm.h>        /* shmat(), IPC_RMID        */
#include <errno.h>          /* errno, ECHILD            */
#include <fcntl.h>          /* O_CREAT, O_EXEC          */
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <signal.h>


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

void prueba_fork2() {

    int cantidad_procesos;
	
    pid_t pid;
    sleep(2);
    for (int j = 0; j < 100; j++) {
        if (pid != 0) {
            pid = fork();

            if (pid != 0) {
                pid = fork();
            }
        }
        else {
            break;
        }
    }
    
    if (pid < 0) { // ocurrió un error
        fprintf(stderr, "Fork fallo"); 
    }
    else if (pid == 0) { // soy el proceso hijo
        // execlp("/bin/ls", "ls", NULL);
        // sleep(3); // hacer el hijo huerfano
        printf("\nHijo pid: %d, ppid: %d\n", getpid(), getppid());
    }
    else { // soy el proceso padre
        while(wait(NULL) > 0); // esperar por el ultimo creado
        printf("Padre pid: %d, ppid: %d\n", getpid(), getppid());
    }


}

float getRand(int range)
{
	float r = ((float)rand() / 15) * range;
	return r;
}

float distribucionNormal(){
    srand(time(NULL));  // Reiniciar la semilla de rand()
    int i = 1; float aux;

    int CANTMUESTRAS = 1000;
    int VARIANZA = 1;
    int MEDIA = 0;
    int RANGO = 1;

    for(i; i <= CANTMUESTRAS; i++){
        aux += (float)rand()/RAND_MAX;
    }
    return fabs(VARIANZA * sqrt((float)12/CANTMUESTRAS) * (aux - (float)CANTMUESTRAS/2) + MEDIA) * RANGO;
}
  
float randomFloat(float min,float max) {
	return (float) min + (rand()/(float)(RAND_MAX))*(max-min);
}

int randomInt(int min,int max) {
	return (rand() % (max-min)) + min;
}

void main (int argc, char **argv){
	//semaforo();

    //tiempo();

    //pruebaFork();

    //pruebaPipe();

    prueba_fork2();
}