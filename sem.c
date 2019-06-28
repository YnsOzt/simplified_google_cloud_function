#include <stdlib.h>
#include <stdio.h>

#include "sem.h"
#include "utils.h"

int createSemaphore(int SEM_ID, int SEMAPHORE_PERMISSIONS){
	int semid = semget(SEM_ID, 1, IPC_CREAT | SEMAPHORE_PERMISSIONS);
	checkNeg(semid, "Error in the method createSemaphore (semget)");

	union semun arg;
	arg.val = 1;
	int semctlRes = semctl(semid, 0, SETVAL, arg);
	checkNeg(semctlRes, "Error in the method createSemaphore (semctl)");

	return semid;
}

int getSemaphore(int SEM_ID, int SEMAPHORE_PERMISSIONS){
	int semid = semget(SEM_ID, 1, SEMAPHORE_PERMISSIONS);
	checkNeg(semid, "Error in the method getSemaphore (semget)");
	return semid;
}

void operationOnSemaphore(int semid, int value){
	struct sembuf sem; 
	sem.sem_num = 0; 
	sem.sem_op = value; 
	sem.sem_flg = 0;

	int rc = semop(semid, &sem, 1);
	checkNeg(rc, "Error in the method operationOnSemaphore (semop)");
}


void down(int semid){
	operationOnSemaphore(semid, DOWN);
}

void up(int semid){
	operationOnSemaphore(semid, UP);
}

void deleteSemaphore(int semid){
	int res = semctl(semid, 0, IPC_RMID);
	checkNeg(res, "Error in the method deleteSemaphore (semctl)");
}


