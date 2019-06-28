#ifndef _SHAREMEM_H_
#define _SHAREMEM_H_

#include <sys/ipc.h>
#include <sys/shm.h>


//create a shared memory using the SHM_ID and return the pointer to the memory zone
//POST: return a pointer to the memory zone
void* createSharedMemory(int shm_id, int shared_memory_permissions, int size);


//get an already created shared memory using the SHM_ID and return the pointer to the memory zone
//POST: return a pointer to the memory zone;
void* getSharedMemory(int shm_id, int shared_memory_permissions, int size);


//get the id of the shared memory zone using the SHM_ID
//POST: return the id of the sahred memory
int getIdOfSharedMemory(int shm_id, int shared_memory_permissions, int size);


//delete the memory zone associated to the shmid
//PRE: - shmid is the id corresponding to the memory zone to delete
void deleteSharedMemory(int shmid);


#endif
