#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "utils.h"
#include "sem.h"
#include "sharemem.h"
#include "types.h"

int main(int argc, char const *argv[]){
	if(argc != 2){
		printf("Mauvaise utilisation de gstat : gstat n\n");
		exit(EXIT_FAILURE);
	}
	int programNumber = atoi(argv[1]);
	int semId = getSemaphore(SEM_ID, SEMAPHORE_PERMISSIONS);
	down(semId);
	programWrapper* memory = (programWrapper*)getSharedMemory(SHM_ID, SHARED_MEMORY_PERMISSIONS, sizeof(programWrapper));
	program prog = memory->programs[programNumber];
	printf("Programme numéro : %d\n", prog.programNum);
	printf("Nom du programme : %s\n", prog.fileName);
	printf("Erreur : %d\n", prog.errorOccured);
	printf("Nombre d'éxecution : %d\n", prog.executionNumber);
	printf("Temps d'éxecution : %ld\n", prog.executionTime);
	up(semId);
	return 0;
}
