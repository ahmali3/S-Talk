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
const char terminate = '!';
bool terminated = false;
char output[MAXSIZE];
pthread_mutex_t writeMutex;
pthread_cond_t outNotEmpty;
pthread_mutex_t csMutex;
pthread_cond_t notEmpty;

// Setting up the socket by creating it and binding it to the server
int socketSetup(int myPort, struct hostent *remoteMachine, int remotePort)
{
	// Creating socket file descriptor
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	
	// Filling server information
	hostaddr.sin_family = AF_INET;
	hostaddr.sin_addr.s_addr = INADDR_ANY;
	hostaddr.sin_port = htons(myPort);

	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr = *(struct in_addr *)remoteMachine->h_addr;
	cliaddr.sin_port = htons(remotePort);

	// Bind the socket with the server address
	if (bind(sockfd, (const struct sockaddr *)&hostaddr, sizeof(hostaddr)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf("binded succesfully\n");
}

// Reads the input from the keyboard and adds it to a list
void *inputData()
{
	// Used to temporarly store the input  
	char buffer[MAXSIZE];
	while (true)
	{
		// Reads the input from keyboard 
		ssize_t size = read(STDIN_FILENO, buffer, MAXSIZE - 1);
		buffer[size] = '\0';
		size++;

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
		int len = sizeof(cliaddr);
		ssize_t test = sendto(sockfd, (const char *)msg, strlen(msg), 0, (const struct sockaddr *)&cliaddr, len);
		if (test == -1)
		{
			printf("Oh dear, something went wrong with read()! %s %d\n", strerror(errno), errno);
		}

		// Checks if either user has terminated the session by sending '!'
		void *hasTerminated = memchr(msg, terminate, strlen(msg));
		if (hasTerminated != NULL)
		{
			terminated = true;
			printf("A user has terminated the session.\n");
			close(sockfd);
			exit(1);
		}
		// Dealloacates the memory for msg
		free(msg);
	}
}

// Recieves the data sent from remote client and puts in onto a list
void *receiveData()
{
	char buffer[MAXSIZE];
	socklen_t socklen = sizeof(cliaddr);
	while (true)
	{
		if (terminated)
		{
			pthread_exit(NULL);
		}
		else
		{
			// takes the data sent to it
			ssize_t data = recvfrom(sockfd, buffer, MAXSIZE, 0, (struct sockaddr *)&cliaddr, &socklen);

			char *readData = malloc(sizeof(char) * data);
			memcpy(readData, buffer, data);

			pthread_mutex_lock(&writeMutex);
			
			ListPrepend(outputList, readData);
			if (ListCount(outputList) != 0)
			{
				pthread_cond_signal(&outNotEmpty);
			}

			pthread_mutex_unlock(&writeMutex);

			// pthread_mutex_unlock(&writeMutex);
			void *hasTerminated = memchr(readData, terminate, strlen(readData));
			if (hasTerminated != NULL)
			{
				terminated = true;
				printf("A user has terminated the session.\n");
				close(sockfd);
				exit(1);
			}
		}
	}
}

// Takes each message off from the list and and outputs it to the screen
void *printData(){
	while (true)
	{
		if(terminated)
        {
            pthread_exit(NULL);
        }
		pthread_mutex_lock(&writeMutex);

		while(ListCount(outputList) == 0){
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
	printf("Program starts here\n");

	// Puts arguments into variables
	int myPort = atoi(argv[1]);
	int remotePort = atoi(argv[3]);
	struct hostent *remoteMachine;
	struct in_addr **addr_list;
	
	// struct addrinfo hints, *res, *p;
    // int status;
    // char ipstr[INET6_ADDRSTRLEN];
	// char hostname[128];
    // if (argc != 4) {
    //     fprintf(stderr,"usage: showip hostname\n");
    //     return 1;
    // }
    // memset(&hints, 0, sizeof hints);
    // hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    // hints.ai_socktype = SOCK_STREAM;
    // if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
    //     fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    //     return 2;
    // }
	// gethostname(hostname, sizeof(hostname));
	// printf("Your Remote Machine: %s\n", hostname);

	char hostname[128];
	remoteMachine = gethostbyname(argv[2]);
	if (remoteMachine == NULL)
	{
		herror("Wrong hostname");
		return 2;
	}
	gethostname(hostname, sizeof(hostname));
	printf("Your Remote Machine: %s\n", hostname);
	printf("Other User's Remote Machine: %s\n", remoteMachine->h_name);
	
	// Initialize lists
	inputList = ListCreate();
	outputList = ListCreate();

	// Initializes socket
	socketSetup(myPort, remoteMachine, remotePort);

	// Initializes threads
	pthread_mutex_init(&csMutex, NULL);
	pthread_cond_init(&notEmpty, NULL);
	pthread_mutex_init(&writeMutex, NULL);
	pthread_cond_init(&outNotEmpty, NULL);

	// Executing threads
	pthread_t input, send, receive, print;

	int inputThread = pthread_create(&input, NULL, inputData, NULL);
	int sendThread = pthread_create(&send, NULL, sendData, NULL);
	int rcvThread = pthread_create(&receive, NULL, receiveData, NULL);
	int printThread = pthread_create(&print, NULL, printData, NULL);

	// Joining threads
	pthread_join(input, NULL);
	pthread_join(send, NULL);
	pthread_join(receive, NULL);
	pthread_join(print, NULL);

	close(sockfd);
	pthread_exit(NULL);

	return 0;
}