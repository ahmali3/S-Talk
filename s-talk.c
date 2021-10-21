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
LIST* inputList;
LIST* outputList;

pthread_mutex_t csMutex;
pthread_cond_t notEmpty;

//NOTE: Add error handling for send, rec, and bind


// Debugging Function
// static void printList(LIST *pList)
// {
//     if (pList->mynodes == 0)
//         printf("No values in the Node\n");
//     LIST *temp = pList;
//     temp->current = pList->first;
//     for (int i = 0; i < temp->mynodes; i++)
//     {
//         printf(" %s \n", (char*)ListCurr(temp));
//         temp->current++;
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
	//cliaddr.sin_addr = *(struct in_addr*)remoteMachine->h_addr;
	//cliaddr.sin_addr.s_addr = INADDR_ANY;
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

void *inputData() {
	char buffer[MAXSIZE];

	while(true) {

	ssize_t size = read(STDIN_FILENO, buffer, MAXSIZE - 1);
	buffer[size] = '\0';
	size++;

	char *msg = malloc(sizeof(char) * size);
	memcpy(msg, buffer, size);

	pthread_mutex_lock(&csMutex);
	ListPrepend(inputList, msg);
	pthread_mutex_unlock(&csMutex);
	
	ssize_t test = sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr *) &cliaddr, sizeof(struct sockaddr *));
	printf("Oh dear, something went wrong with read()! %s %d\n", strerror(errno), errno);
	printf("sendto = %ld\n", test);
	}
}

void *sendData() {
	while (true) {
		pthread_mutex_lock(&csMutex);
        while (ListCount(inputList) == 0)
        {
            pthread_cond_wait(&notEmpty, &csMutex);
        }

	char* msg = (char*) ListTrim(inputList);
	pthread_mutex_unlock(&csMutex);
	ssize_t test = sendto(sockfd, msg, strlen(msg), 0, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
	printf("sendto = %ld\n", test);
	printf("message sent: %s \n", msg);
	free(msg);
	// add null check for msg
	}
}

static void displayListContents(char message[])
{
    //write the message in the list to the console
    size_t messageSize = strlen(message)+1;
    ssize_t allowedToWrite = write(STDOUT_FILENO,message,messageSize);

    if(allowedToWrite == -1)
    {
        printf("Error has occurred writing to screen\n");
        // pthread_mutex_destroy(&readMutex);
        // pthread_cond_destroy(&sendListNotEmpty);
        close(sockfd);
        exit(-1);
    }
}

void *receiveData() {
	char buffer[MAXSIZE];
	while (true) {
//	printf("message received started: %s \n", buffer);
	int data = recvfrom(sockfd, (char*) buffer, MAXSIZE, 0, (struct sockaddr *) &cliaddr, (socklen_t*) sizeof(cliaddr));
//	printf("message received ended: %s \n", buffer);
	displayListContents(buffer);
	}
}

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

	// Initializes threads
    pthread_mutex_init(&csMutex,NULL);
    pthread_cond_init(&notEmpty,NULL);

	// Executing threads
	pthread_t input, send, receive, print;

	//int inputThread = pthread_create(&input, NULL, inputData, NULL);
	//int sendThread = pthread_create(&send, NULL, sendData, NULL);
//	int rcvThread = pthread_create(&receive, NULL, receiveData, NULL);
	//int printThread = pthread_create(&print, NULL, printData, NULL);
	printf("testing 1\n");
	// Joining threads
	//pthread_join(input, NULL);
	//pthread_join(send, NULL);
	//pthread_join(receive, NULL);
	//pthread_join(print, NULL);

	 inputData();
	// inputData();
	 sendData();
	// sendData();
	// receiveData();
	// receiveData();
	// printList(inputList);
	printf("testing 2\n");
	close(sockfd);
	pthread_exit(NULL);

    return 0;
}