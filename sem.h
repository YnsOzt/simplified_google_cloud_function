#ifndef _SEM_H_
#define _SEM_H_

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define UP 1 //the up operation for a semaphore
#define DOWN -1 //the down operation for a semaphore

//cf man semctl
// This function has three or four arguments, depending on cmd.  
// When there are four, the fourth has the type union semun.  
// The  calling program must define this union as follows:
//union semun {
//   int              val;    /* Value for SETVAL */
//   struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
//   unsigned short  *array;  /* Array for GETALL, SETALL */
//   struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
//};



//Create a semaphore with the "key" SEM_ID
//PRE: -SEM_ID is the id of the semaphore to create
//     -SEMAPHORE_PERMISSIONS are the permissions
//POST: return the id of the created semaphore
int createSemaphore(int SEM_ID, int SEMAPHORE_PERMISSIONS);


//get an already existing semaphore
//PRE: -SEM_ID is the id of the semaphore to get
//     -SEMAPHORE_PERMISSIONS are the permissions
//POST: Returns the id of the semaphore 
int getSemaphore(int SEM_ID, int SEMAPHORE_PERMISSIONS);

//execute an operation (+ or - value) on the semaphore corresponding to the semid 
//PRE: -semid is the id on the semaphore which will be modified
//     -value is the value of the operation
//POST: The semaphore is modified by the value
void operationOnSemaphore(int semid, int value);

//execute a DOWN operation on the semaphore corresponding to the semid
//pre: -semid is the id of the samephore which will be decreased by one
//POST: the samphore value get a -1
void down(int semid);

//execute a UP operation on the semaphore corresponding to the semid
//pre: -semid is the id of the samephore which will be increased by one
//POST: the samphore value get a +1
void up(int semid);

//Delete the semaphore corresponding to the semid
//PRE: - semid the id of the semaphore that'll be deleted
void deleteSemaphore(int semid);

#endif
