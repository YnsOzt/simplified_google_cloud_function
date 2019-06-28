#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sock.h"
#include "utils.h"


int createSocket(){
	int sockid = socket(AF_INET, SOCK_STREAM, 0);
	checkNeg(sockid, "Error in the method createSocket (socket)");
	return sockid;
}

void listenPortSocket(int socketid, int port, int MAX_CLIENT){
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));//initiliaze to 0 every field of addr
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	int res = bind(socketid, (struct sockaddr*) &addr, sizeof(addr));
	checkNeg(res, "Error in the method listenPortSocket (bind)");

	res = listen(socketid, MAX_CLIENT);
	checkNeg(res, "Error in the method listenPortSocket (listen)");
}

int acceptClient(int socketid){
	int clientSocketId = accept(socketid, NULL, NULL);
	checkNeg(clientSocketId, "Error in the method acceptClient (accept)");

	return clientSocketId;
}

int connectToTheServer(int socketid ,const char* address, int port){
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(address);
	int res = connect(socketid, (struct sockaddr*) &addr, sizeof(addr));
	checkNeg(res, "Error in the method connectToTheServer (connect)");
	return socketid;
}


