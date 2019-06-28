#ifndef _TYPES_H_
#define _TYPES_H_

#define SHARED_MEMORY_LENGTH 1000 //number of place in the table
#define MAX_FILE_NAME 256
#define SHM_ID 248
#define SHARED_MEMORY_PERMISSIONS 0666
#define MAX_CLIENT 100
#define SEM_ID 369
#define SEMAPHORE_PERMISSIONS 0666


//for comunicating between pipes
#define HEARTBEAT 1
#define ADD_NEW_PGM 2
#define STOP_RECCURENCE 3


//client --> server
#define ADD_PROGRAM_REQUEST -1
#define EXECUTE_PROGRAM_REQUEST -2


//server --> client
#define PGM_DOESNT_EXISTS -2
#define PGM_DOESNT_COMPILE -1
#define PGM_UNEXPECTED_END 0
#define PGM_EXPECTED_END 1


//size of each bloc that we'll read
#define PROGRAM_READ_BUFFER_SIZE 1024

typedef struct program {
	int programNum;
	char fileName[MAX_FILE_NAME];
	int errorOccured;
	int executionNumber;
	long executionTime;
}program;

typedef struct message{
	int type;
	int programNum;
}message;

typedef struct programWrapper{
	program programs[SHARED_MEMORY_LENGTH];
	int logicalSize;
	int totalExecutionNumber;
}programWrapper;


#endif
