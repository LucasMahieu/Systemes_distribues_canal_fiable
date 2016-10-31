#ifndef __STRUCTURE_H
#define __STRUCTURE_H

#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h> // sleep during x s
#include <signal.h> 

#define SERVER "127.0.0.1"	// IP of the process that receives messages
#define PORT 8888   		//The port on which to listen for incoming data

#define MAX_BUFLEN 4096		//Max length of buffer (for messages)

#define MAXMESSAGE 64		//Maximum messages that we can receive simultaneously

typedef struct sockaddr_in Sockaddr_in;
typedef int Socket;

// not used --------------------- --------------------- --------------------- --------------------- --------------------- ---------------------
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
	int numMessage;		// Number of the message. The first message has a value of 1 for this attribute.
	int numSequence; 	// number of the package in the message. The minimum value is 1.
	int maxSequence; 	//	The message is composed of maxSequence messages to assemble. The minimum value is 1.
} enTete;
// not used --------------------- --------------------- --------------------- --------------------- --------------------- ---------------------

// General packet
typedef struct packet {
	uint32_t source; 			// processus source
	uint64_t numPacket;		// Number of the message. The first message has a value of 1 for this attribute.
	uint8_t ack;					// is ack or not (0 if not ack)
	char message[MAX_BUFLEN];				// message (data)
} Packet;



#endif
