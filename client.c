#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <string.h>
#include <signal.h>

#include "types.h"
#include "utils.h"
#include "sock.h"

#define USER_CHOICE_BUFFER_SIZE 200
#define NB_MAX_RECCURENT_ASK 100

//method that create a child and make the child execute the function timerChildHandler
//PRE: -pipfd is the fd of the pipe
//	   -delay is the ms between each iteration of heartbeat
//POST: return the id of the created child
pid_t forkAndRunTimerChild(int* pipefd,int delay);

//method that create a child and make the child execute the function reccurentChildHandler
//PRE: -pipfd is the fd of the pipe
//	   -address is the address of the server
//	   -port is the port of the server	   
//POST: return the id of the created child
pid_t forkAndRunRecurrentChild(int* pipefd, const char* address, int port);

//method that write a struct message in the pipe corresponding to the fd
//PRE: -fd file descriptor where we want to write the message
//	   -message message to write
void writeMessage(int fd, message message);

//method that read a struct message in the file descriptor
//PRE: -fd file descriptor where we want to read the message
//POST: -return the read message
message readMessage(int fd);

//method that connect to the server and add or execute all the asked program
//PRE : -pipefd is the fd of the pipe
//		-address address of the server
//		-port port of the server
void reccurentChildHandler(int* pipefd, const char* address, int port);

//method that send a heartbeat every delay ms
//PRE: -pipefd is the pipefd
//     -delay is the delay between each heartbeat	   
void timerChildHandler(int* pipefd,int delay);

//Method that send the request to execute and all the necessary informations to execute a program
//PRE: -programNumber id of the program to execute
//	   -socketid is the fd of the socket where we'll write all the informations
void sendExecuteRequest(int socketid, int programNumber);

//Method that sends the request to add a new file in the server
//PRE: -socketid is the fd of the socket where we'll write all the informations
//	   -file is the file where the code to send to the server is stored
void sendAddRequest(int socketid, char* file);

//method that read and print the response of the server
//PRE: -serverSocketId socket where we'll read the assigned programNumber and its
//      compilation errors
void receiveAddReponse(int serverSocketId);

//method that read and print the response of the server
//PRE: -serverSocketId socket where we'll read the informations about the execution
void receiveExecuteResponse(int serverSocketId);



pid_t forkAndRunTimerChild(int* pipefd,int delay){
  int childId = fork();
  checkNeg(childId, "Error [forkAndRunTimerChild]");
  
  // child process
  if (childId == 0) {  
    timerChildHandler(pipefd ,delay);
    exit(EXIT_SUCCESS);
  }
  
  return childId;
}


pid_t forkAndRunRecurrentChild(int* pipefd, const char* address, int port){
  int childId = fork();
  checkNeg(childId, "Error [forkAndRunRecurrentChild]");
  
  // child process
  if (childId == 0) {  
    reccurentChildHandler(pipefd, address , port);
    exit(EXIT_SUCCESS);
  }
  
  return childId;
}



void writeMessage(int fd, message message){
	int ret = write(fd, &message, sizeof(message));
	checkNeg(ret, "Error in the method writeMessage in client.c (write)");
}


message readMessage(int fd){
	message message;
	int ret = read(fd, &message, sizeof(message));
	checkNeg(ret, "Error in the method readMessage in client.c (read)");
	return message;
}


void reccurentChildHandler(int* pipefd, const char* address, int port){
	//close the write because this child will only read
	int ret = close(pipefd[1]);
	checkNeg(ret, "Error in the method reccurentChildHandler in client.c (close pipe child)");

	int programNumberList[NB_MAX_RECCURENT_ASK];
	int logicalSize = 0;

	int cSockId, sSockId;
	while(true){
		message messageRead = readMessage(pipefd[0]); //reat the message
		
		switch(messageRead.type){
			case HEARTBEAT:
				//if we added something in the list otherwhise it's useless to open a connection
				if(logicalSize != 0){ 
					
					for(int i = 0 ; i < logicalSize; i++){
						cSockId = createSocket();
						sSockId = connectToTheServer(cSockId, address, port);
						//send the request
						sendExecuteRequest(sSockId, programNumberList[i]);
						//receive the response of the server
						receiveExecuteResponse(sSockId);
						ret = close(sSockId);
						checkNeg(ret, "Error in the method reccurentChildHandler in client.c (close)");
					}
					printf("-----------------------------------------------------------------------\n");
				}
				break;
			case ADD_NEW_PGM:
				if(logicalSize == NB_MAX_RECCURENT_ASK){//if there's no empty space anymore
					printf("Vous avez atteint la limite de programme à executer en récurrence\n");
				}else{
					programNumberList[logicalSize] = messageRead.programNum;
					logicalSize++;
					printf("le nouveau programme %d a été ajouté à la liste\n", messageRead.programNum);
				}
				break;
			case STOP_RECCURENCE:
				close(pipefd[0]);
				exit(EXIT_SUCCESS);
				break;
		}
	}

}

void timerSigintHandler(int signal){
	exit(EXIT_SUCCESS);
}


void timerChildHandler(int* pipefd,int delay){
	//close the read because this child will only write
	int ret = close(pipefd[0]);
	checkNeg(ret, "Error in the method timerChildHandler in client.c (close pipe child 1)");

	ssigaction(SIGINT, timerSigintHandler);//build the sigaction to stop the child whenever the child receive the SIGINT
	
	message message;
	message.type = HEARTBEAT;
	while(true){
		sleep(delay);
		writeMessage(pipefd[1], message);
	}
	//close the write
	ret = close(pipefd[1]);
	checkNeg(ret, "Error in timerChildHandler client.c (close pipe child 2)");
}


void sendExecuteRequest(int socketid, int programNumber){
	int request = EXECUTE_PROGRAM_REQUEST;
	swrite(socketid, &request, sizeof(int));
	swrite(socketid, &programNumber, sizeof(int));
}


void sendAddRequest(int socketid, char* file){
	//read all the necessary informations
	int fileNameSize = strlen(file);

	int request = ADD_PROGRAM_REQUEST;
	swrite(socketid, &request, sizeof(int));
	swrite(socketid, &fileNameSize, sizeof(int));
	swrite(socketid, file, fileNameSize);

	//reading and sending the content file
	int fileFd = sopen(file, O_RDWR, FILE_MODE);

	char fileContent[PROGRAM_READ_BUFFER_SIZE];
	int nbRead = PROGRAM_READ_BUFFER_SIZE;
	while(nbRead ==  PROGRAM_READ_BUFFER_SIZE){
		nbRead = read(fileFd, fileContent, PROGRAM_READ_BUFFER_SIZE);
		printf("SIZE OF READ : %d\n", nbRead);
		printf("CONTENT OF THE FILE : \n");
		write(1, fileContent, nbRead);
		swrite(socketid, &fileContent, nbRead);
	}
	shutdown(socketid ,SHUT_WR);//close the write
}

void receiveAddReponse(int serverSocketId){
	int programNumber;
	sread(serverSocketId, &programNumber, sizeof(int));
	printf("Le numéro assigné à votre programme est le %d\n", programNumber);

	printf("réponse compilateur :\n");

	char response[PROGRAM_READ_BUFFER_SIZE];
	int nbRead;
	while((nbRead = read(serverSocketId,  response, PROGRAM_READ_BUFFER_SIZE)) != 0){
		write(1, response, nbRead);
	}
}

void receiveExecuteResponse(int serverSocketId){
	int programNumber;
	int programState;
	long executionTime;
	int returnCode;
	sread(serverSocketId, &programNumber, sizeof(int));
	sread(serverSocketId, &programState, sizeof(int));
	sread(serverSocketId, &executionTime, sizeof(long));
	sread(serverSocketId, &returnCode, sizeof(int));
	printf("Programme éxécutée : \n");
	printf("numéro : %d\n", programNumber);
	printf("status du programme : %d\n", programState);
	printf("temps d'éxecution : %ld\n", executionTime);
	printf("code de retour : %d\n", returnCode);
	char output[PROGRAM_READ_BUFFER_SIZE];
	int nbRead;
	while((nbRead = read(serverSocketId, output, PROGRAM_READ_BUFFER_SIZE)) != 0){
		write(1, output, nbRead);
	}
}



int main(int argc, char const *argv[]){
	if(argc != 4){
		printf("Mauvaise utilisation de client : ./client adr port delay\n");
		exit(EXIT_FAILURE);
	}

	const char* address = argv[1];
	int port = atoi(argv[2]);
	int delay = atoi(argv[3]);

	//creating pipes
	int pipefd[2];
	int ret = pipe(pipefd);
	checkNeg(ret, "Error in client.c (pipe)");

	pid_t timerPid = forkAndRunTimerChild(pipefd, delay);//create the timer child
	forkAndRunRecurrentChild(pipefd, address, port);//create the reccurent child

	//PARTIE DU PERE

	char userChoice[USER_CHOICE_BUFFER_SIZE];

	//close the read, because the father will never read in the pipe
	ret = close(pipefd[0]);
	checkNeg(ret, "Error in client.c (close pipe father)");

	int clientSocketId;
	int serverSocketId;

	message message;
	message.type = ADD_NEW_PGM;//pre-set the type to add new program


	while(true){
		printf("\nVoici les commandes à votre disposition : \n");
		printf("+ <chemin d'un fichier C>\n");
		printf("* num\n");
		printf("@ num\n");
		printf("q\n");
		if(fgets(userChoice, sizeof(char) * USER_CHOICE_BUFFER_SIZE, stdin) != NULL){
			userChoice[strlen(userChoice) - 1] = '\0'; //delete de \n
			switch(userChoice[0]){
				case '+':
					clientSocketId = createSocket();
					serverSocketId = connectToTheServer(clientSocketId, address, port);
					//+ XXX  --> we are doing the +2 to just get the XXX
					sendAddRequest(serverSocketId, userChoice+2);
					//receive the response of the server
					receiveAddReponse(serverSocketId);

					ret = close(serverSocketId);
					checkNeg(ret, "Error in the method main in client.c (close)");
				break;
				case '*' :
					//@ XXX  --> we are doing the +2 to just get the XXX
					message.programNum = atoi(userChoice+2);
					writeMessage(pipefd[1], message);
				break;
				case '@' :
					clientSocketId = createSocket();
					serverSocketId = connectToTheServer(clientSocketId, address, port);
					
					//send the request
					sendExecuteRequest(serverSocketId, atoi(userChoice+2));
					//receive the response of the server
					receiveExecuteResponse(serverSocketId);

					ret = close(serverSocketId);
					checkNeg(ret, "Error in the method main in client.c (close)");
				break;
				case 'q':
					message.type = STOP_RECCURENCE; //set the type of message to stop reccurence
					writeMessage(pipefd[1], message);
					close(pipefd[1]);
					kill(timerPid, SIGINT);//send the SIGINT signal to the timer 
					exit(EXIT_SUCCESS);
				default:
					printf("Input incorrecte, réessayez !\n");
				break;
			}
		}
	}
	return 0;
}
