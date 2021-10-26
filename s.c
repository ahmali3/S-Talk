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
#include <errno.h>

#define MAXSIZE 1024

int sockfd;
struct sockaddr_in hostaddr;
struct sockaddr_in cliaddr;
LIST *inputList;
LIST *outputList;

pthread_mutex_t csMutex;
pthread_cond_t notEmpty;

void *inputData()
{

	while (true)
	{
		// ssize_t size = read(STDIN_FILENO, buffer, MAXSIZE - 1);
		// buffer[size] = '\0';
		// size++;

		// char *msg = malloc(sizeof(char) * size);
		// memcpy(msg, buffer, size);

		// pthread_mutex_lock(&csMutex);
		// ListPrepend(inputList, msg);

		// pthread_mutex_unlock(&csMutex);

		// if (ListCount(inputList) != 0)
		// {
		// 	printf("inside if statement\n");
		// 	pthread_cond_signal(&notEmpty);
		// }
		printf("inside inputdata\n");
	}
}

void *sendData()
{
	while (true)
	{
		// while (ListCount(inputList) == 0)
		// {
		// 	printf("inside senddata wait\n");
		// 	pthread_cond_wait(&notEmpty, &csMutex);
		// }
		// printf("inside senddata after while\n");
		// pthread_mutex_lock(&csMutex);
		// pthread_mutex_unlock(&csMutex);

		printf("inside senddata\n");
	}
}

void *receiveData()
{
	char buffer[MAXSIZE];
	while (true)
	{
	//	int data = recvfrom(sockfd, (char *)buffer, MAXSIZE, 0, (struct sockaddr *)&cliaddr, (socklen_t *)sizeof(cliaddr));
		printf("inside receiveData\n");

	}
}

int main(int argc, char *argv[])
{
	// if (argc != 4)
	// {
	// 	printf("Invalid number of arguments\n");
	// 	return 0;
	// }
	// printf("Program starts here\n");

	// // Puts arguments into variables
	// int myPort = atoi(argv[1]);
	// int remotePort = atoi(argv[3]);
	// struct hostent *remoteMachine;
	// remoteMachine = gethostbyname(argv[2]);

	// // Initialize lists
	// inputList = ListCreate();
	// outputList = ListCreate();

	// // Initializes socket
	// socketSetup(myPort, remoteMachine, remotePort);

	// Initializes threads
	pthread_mutex_init(&csMutex, NULL);
	pthread_cond_init(&notEmpty, NULL);
	
	// Executing threads
	pthread_t input, send, receive, print;

	int sendThread = pthread_create(&send, NULL, sendData, NULL);
	int inputThread = pthread_create(&input, NULL, inputData, NULL);
	int rcvThread = pthread_create(&receive, NULL, receiveData, NULL);
	//	int printThread = pthread_create(&print, NULL, printData, NULL);

	// Joining threads
	pthread_join(input, NULL);
	pthread_join(send, NULL);
	pthread_join(receive, NULL);
	// pthread_join(print, NULL);

//	close(sockfd);
	pthread_exit(NULL);

	return 0;
}