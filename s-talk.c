/*
 Socket setup idea provided by https://www.binarytides.com/socket-programming-c-linux-tutorial/
 Pthreads idea provided by Pthreads course tutorial
 List implementation provided by course instructor
*/

#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>

#define MAX_SIZE 1024

int sock;
int threadArr[4];
struct sockaddr_in hostAddr;
struct sockaddr_in clientAddr;

LIST *inputList;
LIST *outputList;

const char terminate = '!';
bool terminated = false;
char output[MAX_SIZE];

pthread_mutex_t writeMutex;
pthread_mutex_t csMutex;
pthread_cond_t outNotEmpty;
pthread_cond_t notEmpty;

// Used to free a list
void freeItem(void *a)
{
	a = NULL;
}

// Setting up the socket
int socketSetup(int myPort, struct hostent *remoteMachine, int remotePort)
{
	// Creating socket file descriptor
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Failed to create socket.");
		exit(1);
	}

	// Filling host and client information
	hostAddr.sin_family = AF_INET;
	hostAddr.sin_addr.s_addr = INADDR_ANY;
	hostAddr.sin_port = htons(myPort);

	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr = *(struct in_addr *)remoteMachine->h_addr;
	clientAddr.sin_port = htons(remotePort);

	// Binding the socket with the host address
	if (bind(sock, (const struct sockaddr *)&hostAddr, sizeof(hostAddr)) < 0)
	{
		perror("Failed to bind.");
		exit(1);
	}
}

// Reads the input from the keyboard and adds it to a list
void *inputData()
{
	// Used to temporarly store the input
	char buffer[MAX_SIZE];
	while (true)
	{
		// Reads the input from keyboard
		ssize_t size = read(STDIN_FILENO, buffer, MAX_SIZE - 1);
		buffer[size++] = '\0';

		char *msg = malloc(sizeof(char) * size);
		memcpy(msg, buffer, size);

		pthread_mutex_lock(&csMutex);

		ListPrepend(inputList, msg);

		// If list is not empty then unblocks the thread that keeps
		// track of if list is empty or not
		if (ListCount(inputList) != 0)
		{
			pthread_cond_signal(&notEmpty);
		}

		pthread_mutex_unlock(&csMutex);
	}
}

// Takes data off the list and sends it to remote client
void *sendData()
{
	while (true)
	{
		pthread_mutex_lock(&csMutex);

		// Waits until the the list is not empty
		while (ListCount(inputList) == 0)
		{
			pthread_cond_wait(&notEmpty, &csMutex);
		}

		// Trims a message off the list to be sent
		char *msg = (char *)ListTrim(inputList);
		pthread_mutex_unlock(&csMutex);

		// Sends the message to remote client
		ssize_t test = sendto(sock, (const char *)msg, strlen(msg), 0, (const struct sockaddr *)&clientAddr, sizeof(clientAddr));
		if (test == -1)
		{
			printf("Error sending message. %s %d\n", strerror(errno), errno);
		}

		// Checks if either user has terminated the session by sending '!'
		void *hasTerminated = memchr(msg, terminate, strlen(msg));
		if (hasTerminated != NULL)
		{
			terminated = true;
			printf("Session has been terminated.\n");

			// Destroying mutexes and condition variables
			pthread_mutex_unlock(&csMutex);
			pthread_mutex_unlock(&writeMutex);
			pthread_mutex_destroy(&csMutex);
			pthread_mutex_destroy(&writeMutex);

			pthread_cond_signal(&notEmpty);
			pthread_cond_signal(&outNotEmpty);
			pthread_cond_destroy(&notEmpty);
			pthread_cond_destroy(&outNotEmpty);

			ListFree(inputList, freeItem);
			ListFree(outputList, freeItem);

			close(sock);
			free(msg);

			exit(1);
		}
		free(msg);
	}
}

// Receives the data sent from remote client and puts in into a list
void *receiveData()
{
	char buffer[MAX_SIZE];
	socklen_t socklen = sizeof(clientAddr);
	while (true)
	{
		if (terminated)
		{
			pthread_exit(NULL);
		}
		else
		{
			// Takes the data sent to it
			ssize_t data = recvfrom(sock, buffer, MAX_SIZE, 0, (struct sockaddr *)&clientAddr, &socklen);

			char *readData = malloc(sizeof(char) * data);
			memcpy(readData, buffer, data);

			pthread_mutex_lock(&writeMutex);

			ListPrepend(outputList, readData);
			if (ListCount(outputList) != 0)
			{
				pthread_cond_signal(&outNotEmpty);
			}

			pthread_mutex_unlock(&writeMutex);

			void *hasTerminated = memchr(readData, terminate, strlen(readData));
			if (hasTerminated != NULL)
			{
				terminated = true;

				ListFree(inputList, freeItem);
				ListFree(outputList, freeItem);

				printf("User has terminated the session.\n");

				// Destroying mutexes and condition variables
				pthread_mutex_unlock(&csMutex);
				pthread_mutex_unlock(&writeMutex);
				pthread_mutex_destroy(&csMutex);
				pthread_mutex_destroy(&writeMutex);

				pthread_cond_signal(&notEmpty);
				pthread_cond_signal(&outNotEmpty);
				pthread_cond_destroy(&notEmpty);
				pthread_cond_destroy(&outNotEmpty);

				close(sock);
				exit(1);
			}
		}
	}
}

// Takes each message off from the list and and outputs it to the screen
void *printData()
{
	while (true)
	{
		if (terminated)
		{
			pthread_exit(NULL);
		}
		pthread_mutex_lock(&writeMutex);

		while (ListCount(outputList) == 0)
		{
			pthread_cond_wait(&outNotEmpty, &writeMutex);
		}
		char *outputMessage = ListTrim(outputList);
		size_t outputSize = strlen(outputMessage);

		pthread_mutex_unlock(&writeMutex);

		ssize_t writeOutput = write(STDOUT_FILENO, outputMessage, outputSize);
		free(outputMessage);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		printf("Invalid number of arguments\n");
		return 0;
	}

	// Puts arguments into variables
	int myPort = atoi(argv[1]);
	int remotePort = atoi(argv[3]);
	struct hostent *remoteMachine;
	struct in_addr **addr_list;

	char hostname[128];
	remoteMachine = gethostbyname(argv[2]);
	if (remoteMachine == NULL)
	{
		herror("Wrong hostname");
		return 2;
	}
	gethostname(hostname, sizeof(hostname));
	printf("Your Machine Name: %s\n", hostname);
	printf("Other User's Machine Name: %s\n", remoteMachine->h_name);

	// Initialize lists
	inputList = ListCreate();
	outputList = ListCreate();

	// Initializes socket
	socketSetup(myPort, remoteMachine, remotePort);

	// Initializes threads
	pthread_mutex_init(&csMutex, NULL);
	pthread_mutex_init(&writeMutex, NULL);
	pthread_cond_init(&notEmpty, NULL);
	pthread_cond_init(&outNotEmpty, NULL);

	// Executing threads
	pthread_t input, send, receive, print;

	threadArr[0] = pthread_create(&input, NULL, inputData, NULL);
	threadArr[1] = pthread_create(&send, NULL, sendData, NULL);
	threadArr[2] = pthread_create(&receive, NULL, receiveData, NULL);
	threadArr[3] = pthread_create(&print, NULL, printData, NULL);

	// Joining threads
	pthread_join(input, NULL);
	pthread_join(send, NULL);
	pthread_join(receive, NULL);
	pthread_join(print, NULL);

	// Destroying mutexes and condition variables
	pthread_mutex_unlock(&csMutex);
	pthread_mutex_unlock(&writeMutex);
	pthread_mutex_destroy(&csMutex);
	pthread_mutex_destroy(&writeMutex);
	pthread_cond_destroy(&notEmpty);
	pthread_cond_destroy(&outNotEmpty);

	ListFree(inputList, freeItem);
	ListFree(outputList, freeItem);

	close(sock);

	return 0;
}