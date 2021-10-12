// Server side implementation of UDP client-server model
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <netdb.h>
#include <pthread.h>

#define MAXLINE 1024

int sockfd;
struct sockaddr_in hostaddr;
struct sockaddr_in cliaddr;
LIST* inputList;
LIST* outputList;

// static void printList(LIST *pList)
// {
//     if (pList->mynodes == 0)
//         printf("No values in the Node\n");
//     LIST *temp = pList;
//     temp->current = pList->first;
//     while (temp->current != NULL && pList->mynodes != 0)
//     {
//         printf(" %d ", *(int *)List_curr(temp));
//         temp->curitem = temp->curitem;
//     }
//     printf("\n");
// }

int socketSetup(int myPort, struct hostent *remoteMachine,int remotePort){
	
	// Creating socket file descriptor
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	
	// Filling server information
	hostaddr.sin_family = AF_INET;
	hostaddr.sin_addr.s_addr = INADDR_ANY;
	hostaddr.sin_port = htons(myPort);

	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr = *(struct in_addr*)remoteMachine->h_addr;
	cliaddr.sin_port = htons(remotePort);
	
	memset(&hostaddr, 0, sizeof(hostaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&hostaddr, sizeof(hostaddr)) < 0 )
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf("binded succesfully\n");
}

int inputData() {
	char buffer[MAXLINE];
	ssize_t size = read(STDIN_FILENO, buffer, MAXLINE - 1);
	buffer[size++] = '\0';
	char *msg = malloc(sizeof(char) * size);
	memcpy(msg, buffer, size);
	ListPrepend(inputList, msg);
}

// int sendData() {
// 	int len, n;

// 	len = sizeof(cliaddr); //len is value/resuslt
	
// 	n = recvfrom(sockfd, (char *)buffer, MAXLINE,
// 				MSG_WAITALL, ( struct sockaddr *) &cliaddr, (socklen_t*) &len);
// 	buffer[n] = '\0';
// 	printf("Client : %s\n", buffer);
// 	sendto(sockfd, (const char *)hello, strlen(hello),MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
// 	printf("Hello message sent.\n");
	
// 	return 0;
// }

// int receiveData() {
// 	char *hello = "Hello from client";
// 	struct sockaddr_in	 servaddr;

// 	memset(&servaddr, 0, sizeof(servaddr));
	
// 	// Filling server information
// 	servaddr.sin_family = AF_INET;
// 	servaddr.sin_port = htons(PORT);
// 	servaddr.sin_addr.s_addr = INADDR_ANY;
	
// 	int n, len;
	
// 	sendto(sockfd, (const char *)hello, strlen(hello),
// 		MSG_CONFIRM, (const struct sockaddr *) &servaddr,
// 			sizeof(servaddr));
// 	printf("Hello message sent.\n");
		
// 	n = recvfrom(sockfd, (char *)buffer, MAXLINE,
// 				MSG_WAITALL, (struct sockaddr *) &servaddr, (socklen_t*) &len);
// 	buffer[n] = '\0';
// 	printf("Server : %s\n", buffer);

// 	close(sockfd);
// 	return 0;
// }

int main(int argc, char *argv[]){
	if (argc != 4) {
		printf("Invalid number of arguments\n");
		return 0;
	}
	printf("Program starts here\n");

	// Puts arguments into variables
	int myPort = atoi(argv[1]);
	int remotePort = atoi(argv[3]);
	struct hostent *remoteMachine;
	remoteMachine = gethostbyname(argv[2]);

	// Initialize lists
	inputList = ListCreate();
	outputList = ListCreate();

	// Initializes socket
	socketSetup(myPort, remoteMachine, remotePort);

	inputData();
	inputData();
//	printList(inputList);
	printf(" %s \n", (char*)ListCurr(inputList));
    return 0;
}