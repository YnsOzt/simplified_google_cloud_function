#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sharemem.h"
#include "utils.h"

void* createSharedMemory(int shm_id, int shared_memory_permissions, int size){
	int shmid = shmget(shm_id, size, IPC_CREAT | shared_memory_permissions);
	checkNeg(shmid, "Error in the method createSharedMemory (shmget)");
	
	void* memoryZone = shmat(shmid, NULL, 0);

	return memoryZone;
}

void* getSharedMemory(int shm_id, int shared_memory_permissions, int size){
	int shmid = shmget(shm_id, size, shared_memory_permissions);
	checkNeg(shmid, "Error in the method getSharedMemory (shmget)");

	void* memoryZone = shmat(shmid, NULL, 0);
	//checkNeg(*memoryZone, "Error in the method getSharedMemory (shmat)");

	return memoryZone;
}

int getIdOfSharedMemory(int shm_id, int shared_memory_permissions, int size){
	int shmid = shmget(shm_id, size, shared_memory_permissions);
	checkNeg(shmid, "Error in the method getIdOfSharedMemory (shmget)");
	return shmid;
}

void deleteSharedMemory(int shmid){
	int res = shmctl(shmid, IPC_RMID, NULL);
	checkNeg(res, "Error in the method deleteSharedMemory (shmctl)");
}




