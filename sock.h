#ifndef _SOCK_H_
#define _SOCK_H_

/* Used by server and client */
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
/* struct sockaddr_in */
#include <netinet/in.h>
#include <arpa/inet.h>


//create a socket and returns it fd
//POST: return the fd of the created socket
int createSocket();

//make the socket corresponding to the socketid listen the port given in parameter
//PRE: -socketid is the socketid corresponding to the socket that'll listen to the port
//     -port is the port that the socket will listen
//     -max_client maximum accepted client
void listenPortSocket(int socketid, int port, int max_client);


//accept the client for the socket corresponding to the given socketid
//PRE: -socketid is the socket that'll accept the client
//POST: -returns the id of the client's socket
int acceptClient(int socketid);


//Connect the client and the server using TCP protocol
//PRE: - socketid is the socketid of the client
//     - address is the address of the server
//     - port is the port of the server
//POST: return the socketid of the server
int connectToTheServer(int socketid ,const char* address, int port);


#endif
