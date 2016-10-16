#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h> // sleep during x s
#include <signal.h> 

typedef struct listeChaine {
	int num;
	struct listeChaine* next;
} listeChaine;

// Structure for the Tab to get informations about the message stored in this element
typedef struct IDMessage {
	int numMessage; 	// Number of the message 
	int maxSequence;	// The message is composed of max messages to assemble
	listeChaine list; 	// List of already received package numbers
	char* buffer;		// Where the message is stored
} IDMessage;

// En tete of a udp package
typedef struct enTete {
	int numMessage;		// Number of the message 
	int numSequence; 	// number of the package in the message
	int maxSequence; 	//	The message is composed of maxSequence messages to assemble
} enTete;

#define BUFLEN 512  		//Max length of buffer
#define MAXMESSAGE 64		//Maximum messages that we can receive simultaneously
IDMessage Tab[MAXMESSAGE];	// Array of temporarily stored messages