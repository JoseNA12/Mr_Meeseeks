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

#include "Proceso.c"
#include "Vector.c"

struct proceso *procesoNuevo;
static vector *lista_procesos;
//static vector *lista_de_listas;

void pruebaForks() {

    lista_procesos = (struct vector*)malloc(sizeof(struct vector));
    //lista_de_listas = (struct vector*)malloc(sizeof(struct vector));
    procesoNuevo = (struct proceso*)malloc(sizeof(struct proceso));

    vector_free(lista_procesos); // limpiar la lista
    vector_init(lista_procesos);
    //vector_free(lista_de_listas); // limpiar la lista
    //vector_init(lista_de_listas);

//  =====================================================================================================================================
    procesoNuevo = (struct proceso*)malloc(sizeof(struct proceso));
    *procesoNuevo = (struct proceso){.nivel = 1, .pid = 1, .ppid = 60};
    vector_add(lista_procesos, procesoNuevo);
    //vector_add(lista_de_listas, lista_procesos);
// ==========================================================LIMPIAR=====================================================================
    //lista_procesos = (struct vector*)malloc(sizeof(struct vector));
    //vector_free(lista_procesos); // limpiar la lista
    //vector_init(lista_procesos);
// =====================================================================================================================================
    procesoNuevo = (struct proceso*)malloc(sizeof(struct proceso));
    *procesoNuevo = (struct proceso){.nivel = 2, .pid = 22, .ppid = 66};
    vector_add(lista_procesos, procesoNuevo);
    //vector_add(lista_de_listas, lista_procesos);
// ==========================================================LIMPIAR=====================================================================
    //lista_procesos = (struct vector*)malloc(sizeof(struct vector));
    //vector_free(lista_procesos); // limpiar la lista
    //vector_init(lista_procesos);
// =====================================================================================================================================
    procesoNuevo = (struct proceso*)malloc(sizeof(struct proceso));
    *procesoNuevo = (struct proceso){.nivel = 3, .pid = 33, .ppid = 80};
    vector_add(lista_procesos, procesoNuevo);
    //vector_add(lista_de_listas, lista_procesos);

    //printf("Tamaño lista GIGANTE %d \n",vector_total(lista_de_listas));
    printf("===================================================================== \n");


//  [   [ proceso, proceso, proceso ], [ proceso, proceso ] ]

    //for (int i = 0; i < vector_total(lista_de_listas); i++) { // Recorrer lista gigante

        //struct vector *lista_seleccionada = (struct vector*) vector_get(lista_de_listas, i);
        printf("Tamaño lista PEQUEÑA %d",vector_total(lista_procesos));

        for (int j = 0; j < vector_total(lista_procesos); j++) { // Recorrer lista pequeña

            struct proceso *proceso_seleccionada = (struct proceso*) vector_get(lista_procesos, j);
            printf("\n  NIVEL: %d",proceso_seleccionada->nivel);
            printf("\n  PID: %d",proceso_seleccionada->pid);
            printf("\n  PPID: %d \n " ,proceso_seleccionada->ppid);  
            printf("===================================================================== \n");
        }              
    //}

    //vector_free(lista_de_listas); // limpiar la lista
    //vector_init(lista_de_listas);
    printf("Tamaño lista GIGANTE %d \n",vector_total(lista_procesos));
}


int main (int argc, char **argv){
	pruebaForks();
}