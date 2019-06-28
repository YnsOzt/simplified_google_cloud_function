#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>

#include "utils.h"
#include "sem.h"
#include "sharemem.h"
#include "sock.h"
#include "types.h"

#define NEW_PGM_FILE_SIZE 150
#define OUTPUT_FILE_NAME_SIZE 150
#define PERM 0666
#define NEW_PGM_PATH "./code_repository"
#define COMPILE_OUTPUT_PATH "./compiler_output"
#define EXECUTION_OUTPUT_PATH "./outputs"

//method that execute the method executeProtocolHandler
//PRE: -clientSocketId fd of the socket where we'll read all the informations
//     -n pointer to the integer where we'll write the program id that we'll execute
//	   -semid semaphore id
//     -memoryZone shared memory
void executeProtocolHandler(int clientSocketId, int semid, programWrapper* memoryZone);

//fork of the method execl to compile the program
//PRE: -programName name of the program to execute
pid_t forkAndRunExeclRun(char* programName);

//Method that sends the informations about an execution to the client
//PRE: -clientSocketid socket where we'll write all the informations
//	   -programNum number of the program
//     -programState state of the program
//     -execTime execution time of the program
//     -programStatus status of the program
//     -outputFileName name of the output file
void sendExecutionResponse(int clientSocketId, int programNum, int programState, long execTime, int programStatus, char* outputFileName);

//PRE: -clientSocketId fd of the socket where we'll read all the informations
//	   -semid is the id of the semaphore
//     -memoryZone is the shared memory
void addProtocolHandler(int clientSocketId, int semid, programWrapper* memoryZone);

//method that add a new program in the shared memory
//PRE: -semid the id of the semaphore to access the shared memory
//	   -memoryZone is a pointer to the shared memory
//	   -fileName name of the file to add
//POST: returns -1 if there's no empty space otherwhise the number of the program on the shared memory
int createProgramInSharedMem(int semid, programWrapper* memoryZone, char* fileName);
   
//method that fill the file by reading the user's program in the given socket
//PRE: -clientSocketId is the socket where we'll read the program of the client
//	   -newFileFd is the file descriptor of the file where we'll write the read program
void fillFileWithUserProgram(int clientSocketId, FILE* newFileFd);

//method that compile a program
//PRE: -outputPgmName is the name of the output when we'll compile the program (exec)
//     -pgmFileName name of the program (.c)
//     -outputPgmFileName  number that represent the outputFileName
//     -memoryZone is the sahred memory
void compileProgram(char* outputPgmName, char* pgmFileName, int outputPgmFileName, programWrapper* memoryZone);

//fork of the method execl to compile the program
//PRE: -outputPgmName is the name of the output when we'll compile the program (exec)
//     -pgmFileName name of the program (.c)
pid_t forkAndRunExeclCompile(char* outputPgmName, char* pgmFileName);

//method that send the reponse to the client
//PRE: -clientSocketId socket where we'll write the response
//	   -programNumber number of the program and also the name of the outputfile
void sendAddResponse(int clientSocketId, int programNumber);

//method that create a child that treat the clients request
//PRE: -clientSocketId is the socket where we'll read all the informations
//     -semid is the semaphore to access the shared memory
//     -memoryZone is the shared memory
void forkAndRunTreatClient(int clientSocketId, int semid, programWrapper* memoryZone);

void executeProtocolHandler(int clientSocketId, int semid, programWrapper* memoryZone){
	int programNum;
	sread(clientSocketId, &programNum, sizeof(int));

	int programState;

	//get the logicalSize of the sharedmemory
	down(semid);
	int logicalSize = memoryZone->logicalSize;
	if(logicalSize < programNum){//if the program doesn't exists
		programState = PGM_DOESNT_EXISTS;
	}else if(memoryZone->programs[programNum].errorOccured == 0){//if an error occured during the compilation
		programState = PGM_DOESNT_COMPILE;
	}
	//get the total number of execution to assign it as the fileName
	int totalExecution = memoryZone->totalExecutionNumber;
	memoryZone->totalExecutionNumber++;
	up(semid);

	//create the output file
	char programName[OUTPUT_FILE_NAME_SIZE]; //name of the program to execute
	char outputFileName[OUTPUT_FILE_NAME_SIZE]; //output file name 
	sprintf(programName, "%d", programNum);
	sprintf(outputFileName, "%s/%d.txt", EXECUTION_OUTPUT_PATH, totalExecution);
	int fd = open(outputFileName, O_CREAT | O_WRONLY| O_TRUNC, PERM);
	checkNeg(fd, "ERROR open in server.c (executeProtocolHandler)");

	int stdout_copy = dup(1);
 	checkNeg(stdout_copy, "ERROR dup in server.c (executeProtocolHandler)");

 	int ret = dup2(fd, 1);
  	checkNeg(ret, "ERROR dup2 in server.c (executeProtocolHandler)");

	long t1 = now();
	forkAndRunExeclRun(programName);//run the already compiled program
	int status;
	wait(&status);
	long t2 = now();
	int programStatus = 0;
	

	long execTime = 0;
	if(WIFEXITED(status)){
		//if there's not an error after the run and state not already setted before
		if(WEXITSTATUS(status) != EXIT_FAILURE && programState != PGM_DOESNT_EXISTS && programState != PGM_DOESNT_COMPILE){
			programState = PGM_EXPECTED_END;
			programStatus = WEXITSTATUS(status);
			//update the program in the shared memory
	  		execTime = (t2 - t1);
			down(semid);
			memoryZone->programs[programNum].executionNumber++;
			memoryZone->programs[programNum].executionTime += execTime;
			up(semid);
		}else if(programState != PGM_DOESNT_EXISTS && programState != PGM_DOESNT_COMPILE){
			programState = PGM_UNEXPECTED_END;
		}
	}

	ret = dup2(stdout_copy, 1);
  	checkNeg(ret, "ERROR dup2 (no2) in server.c (executeProtocolHandler)");
  	close(stdout_copy);

	//sends the response to the client
  	sendExecutionResponse(clientSocketId, programNum, programState, execTime, programStatus, outputFileName);
}

pid_t forkAndRunExeclRun(char* programName){
	int childId = fork();
  	checkNeg(childId, "Error [forkAndRunExeclRun]");
  
	// child process
	if (childId == 0) {
	  	char firstArg[OUTPUT_FILE_NAME_SIZE];
	  	sprintf(firstArg, "%s/./%s", NEW_PGM_PATH, programName);
	  	execl(firstArg, programName, NULL);
	 	exit(EXIT_FAILURE); //if an error occured during execl
	}
    return childId;
}

void sendExecutionResponse(int clientSocketId, int programNum, int programState, long execTime, int programStatus, char *outputFileName){
	swrite(clientSocketId, &programNum, sizeof(int));
	swrite(clientSocketId, &programState, sizeof(int));
	swrite(clientSocketId, &execTime, sizeof(long));
	swrite(clientSocketId, &programStatus, sizeof(int));
	int fileFd = sopen(outputFileName, O_RDWR, PERM);
	char fileContent[PROGRAM_READ_BUFFER_SIZE];
	int nbRead = PROGRAM_READ_BUFFER_SIZE;
	while(nbRead ==  PROGRAM_READ_BUFFER_SIZE){
		nbRead = read(fileFd,&fileContent, PROGRAM_READ_BUFFER_SIZE);
		swrite(clientSocketId, &fileContent, nbRead);
	}

	shutdown(clientSocketId, SHUT_WR);//close the write
	//close the socket of the client
	int ret = close(clientSocketId);
	checkNeg(ret, "ERROR in server.c (close in executeProtocolHandler)");
}

void addProtocolHandler(int clientSocketId, int semid, programWrapper* memoryZone){
	int nbCharFileName = 0;
	char fileName[MAX_FILE_NAME];
	sread(clientSocketId, &nbCharFileName, sizeof(int));
	sread(clientSocketId, &fileName, nbCharFileName);

	int programNumber = createProgramInSharedMem(semid, memoryZone, fileName);

	if(programNumber != -1){//if the add is sucessfully done
		//extract the new file
		char newFileName[NEW_PGM_FILE_SIZE];
		sprintf(newFileName, "%s/%d.c", NEW_PGM_PATH, programNumber);
		FILE* newFileFd = fopen(newFileName, "w");
		checkNull(newFileFd, "Error in server.c (fopen in addProtocol)");
		fillFileWithUserProgram(clientSocketId, newFileFd);
		fclose(newFileFd);

		//compile the program
		char outputFileName[OUTPUT_FILE_NAME_SIZE];
		sprintf(outputFileName, "%s/%d", NEW_PGM_PATH, programNumber);

		compileProgram(outputFileName, newFileName, programNumber, memoryZone);
		sendAddResponse(clientSocketId, programNumber);
	}
}

int createProgramInSharedMem(int semid, programWrapper* memoryZone, char* fileName){
	down(semid);
	//if not enough place in the server's source code repository
	if(memoryZone->logicalSize == SHARED_MEMORY_LENGTH){
		printf("L'ajout ne peut être fiat car il n'y a plus de place en mémoire\n");
		up(semid);
		return -1;
	}

	//initialize the new program and it's name
	program pgm;
	memset(&pgm, 0, sizeof(pgm)); //initialise every field to 0
	int programNumber = memoryZone->logicalSize;
	strcpy(pgm.fileName, fileName);
	pgm.programNum = programNumber;
	memoryZone->programs[memoryZone->logicalSize] = pgm;
	memoryZone->logicalSize++; //increment the logicalSize of our shared memory's array

	up(semid);
	return programNumber;
}

void fillFileWithUserProgram(int clientSocketId, FILE* newFileFd){
	char fileContent[PROGRAM_READ_BUFFER_SIZE];
	int nbRead;
	while((nbRead = read(clientSocketId, &fileContent, PROGRAM_READ_BUFFER_SIZE)) != 0){
		printf("SIZE OF READ : %d\n", nbRead);
		printf("CONTENT OF THE FILE : %s\n", fileContent);
		fprintf(newFileFd, "%s", fileContent);
	}
}

pid_t forkAndRunExeclCompile(char* outputPgmName, char* pgmFileName){
	int childId = fork();
  	checkNeg(childId, "Error [forkAndRunExeclCompile]");
  
	// child process
	if (childId == 0) {
	  	int res = execl("/usr/bin/gcc", "gcc", "-o", outputPgmName, pgmFileName, NULL);
		checkNeg(res, "Error in server.c (execl in compileProgram)");
	  	exit(EXIT_SUCCESS);
	}

    return childId;
}

void compileProgram(char* outputPgmName, char* pgmFileName, int outputPgmFileName, programWrapper* memoryZone){
	char outputFileName[OUTPUT_FILE_NAME_SIZE];
	sprintf(outputFileName, "%s/%d.txt", COMPILE_OUTPUT_PATH, outputPgmFileName);
	int fd = open(outputFileName, O_CREAT | O_WRONLY| O_TRUNC, PERM);
	checkNeg(fd, "ERROR open in server.c (compileProgram)");

	int stderr_copy = dup(2);
 	checkNeg(stderr_copy, "ERROR dup in server.c (compileProgram)");

 	int ret = dup2(fd, 2);
  	checkNeg(ret, "ERROR dup2 in server.c (compileProgram)");

  	forkAndRunExeclCompile(outputPgmName, pgmFileName);
  	int status;
  	wait(&status);

  	if(WEXITSTATUS(status) != 0){ //if an error has occured during the compilation
  		memoryZone->programs[outputPgmFileName].errorOccured = 0;
  	}else{
  		memoryZone->programs[outputPgmFileName].errorOccured = 1;
  	}

  	ret = dup2(stderr_copy, 2);
  	checkNeg(ret, "ERROR dup2 (no2) in server.c (compileProgram)");
  	close(stderr_copy);
}

void sendAddResponse(int clientSocketId, int programNumber){
	//send the program number
	swrite(clientSocketId, &programNumber, sizeof(int));

	//send the compilation errors
	char file[OUTPUT_FILE_NAME_SIZE];
	sprintf(file, "%s/%d.txt", COMPILE_OUTPUT_PATH, programNumber);
	int fileFd = sopen(file, O_RDWR, PERM);

	char fileContent[PROGRAM_READ_BUFFER_SIZE];
	int nbRead = PROGRAM_READ_BUFFER_SIZE;
	while(nbRead ==  PROGRAM_READ_BUFFER_SIZE){
		nbRead = read(fileFd,&fileContent, PROGRAM_READ_BUFFER_SIZE);
		swrite(clientSocketId, &fileContent, nbRead);
	}
	shutdown(clientSocketId, SHUT_WR);//close the write
	//close the socket of the client
	int ret = close(clientSocketId);
	checkNeg(ret, "Error in server.c (close socket)");
}

void forkAndRunTreatClient(int clientSocketId, int semid, programWrapper* memoryZone){
	int childId = fork();
  	checkNeg(childId, "Error [forkAndRunTreatClient]");
  
	// child process
	if (childId == 0) {
		signal(SIGINT, SIG_IGN);//ignre the SIGINT signal to end it's job before exiting
		int clientRequest;
		sread(clientSocketId, &clientRequest, sizeof(int));
		switch(clientRequest){
			case EXECUTE_PROGRAM_REQUEST:
				executeProtocolHandler(clientSocketId, semid, memoryZone);
			break;
			case ADD_PROGRAM_REQUEST:
				addProtocolHandler(clientSocketId, semid, memoryZone);
			break;
		}
		exit(EXIT_SUCCESS);//to kill the process after the execution
	}
	
}


int main(int argc, char const *argv[]){
	if(argc != 2){
		printf("Mauvaise utilisation de server : ./server port \n");
		exit(EXIT_FAILURE);
	}
	int port = atoi(argv[1]);

	signal(SIGCHLD, SIG_IGN); //IGNORE THE SIGCHILD to kill the zombies after the execution of the server

	int serverSocketId = createSocket();
	listenPortSocket(serverSocketId, port, MAX_CLIENT);

	//get the semaphore and shared memory id
	int semid = getSemaphore(SEM_ID, SEMAPHORE_PERMISSIONS);
	programWrapper* memoryZone = (programWrapper*)getSharedMemory(SHM_ID, SHARED_MEMORY_PERMISSIONS,sizeof(programWrapper));

	int clientSocketId;

	while(true){
		clientSocketId = acceptClient(serverSocketId);
		forkAndRunTreatClient(clientSocketId, semid, memoryZone);
	}

	return 0;
}
