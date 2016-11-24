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

// General packet
typedef struct packet {
	uint32_t source; 			// processus source
	uint64_t numPacket;		// Number of the message. The first message has a value of 1 for this attribute.
	uint8_t ack;					// is ack or not (0 if not ack)
	uint32_t size;
	char message[MAX_BUFLEN];				// message (data)
} Packet;

#endif
