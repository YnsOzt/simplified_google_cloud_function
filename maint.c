#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "utils.h"
#include "sem.h"
#include "sharemem.h"
#include "types.h"

void createResourcesForSharedMemory(){
	createSemaphore(SEM_ID, SEMAPHORE_PERMISSIONS);
	createSharedMemory(SHM_ID, SHARED_MEMORY_PERMISSIONS, sizeof(programWrapper));
	printf("CREATION DES RESSOURCES TERMINEE CORRECTEMENT\n");
}

void destroyResourcesOfSharedMemory(){
	int shmid = getIdOfSharedMemory(SHM_ID, SHARED_MEMORY_PERMISSIONS, sizeof(programWrapper));
	deleteSharedMemory(shmid);
	int semid = getSemaphore(SEM_ID,SEMAPHORE_PERMISSIONS);
	deleteSemaphore(semid);
	printf("DESTRUCTION DES RESSOURCES TERMINEE CORRECTEMENT\n");
}

void useExclusivelySharedMemory(int seconde){
	int semid = getSemaphore(SEM_ID,SEMAPHORE_PERMISSIONS);
	down(semid);
	sleep(seconde);
	up(semid);
}

int main(int argc, char const *argv[]){
	if(argc > 1 && argc < 4){
		int type;
		int opt; 
		if(argc == 3){
			opt = atoi(argv[2]);
		}
		if(argc <= 3){
			type = atoi(argv[1]);
			switch(type){
				case 1:
					createResourcesForSharedMemory();
					break;
				case 2:
					destroyResourcesOfSharedMemory();
					break;
				case 3:
					if(argc == 3){
						useExclusivelySharedMemory(opt);
					}else{
						printf("./maint type [opt] si votre type = 3 --> vous devez introduire l'opt\n");
						exit(EXIT_FAILURE);
					}
					break;
				default:
					printf("./maint type [opt]  où type doit être compris entre 1 et 3\n");
					exit(EXIT_FAILURE);
			}
		}
	}else{
		printf("Problème avec le format du programme voici son utilisation : ./maint type [opt]\n");
		exit(EXIT_FAILURE);
	}
}


