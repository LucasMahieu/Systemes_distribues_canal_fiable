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

// IP of the process that receives messages
#define SERVER "127.0.0.1"	
//The port on which to listen for incoming data
#define PORT 8888

//Max length of buffer (for messages)
#define MAX_MES_LEN 8000
#define MAX_PKT_LEN (MAX_MES_LEN+sizeof(uint32_t)+sizeof(uint8_t))

typedef struct sockaddr_in Sockaddr_in;
typedef int Socket;

// General packet
typedef struct packet {
	uint32_t numPacket;		// Number of the message. The first message has a value of 1 for this attribute.
	uint8_t ack;					// is ack or not (0 if not ack)
	char message[MAX_MES_LEN];				// message (data)
} Packet;

char *serialize_packet(char *buffer, Packet *p);
Packet *deserialize_packet(char *buffer, Packet *p);
char *serialize_uint32(char *buffer, uint32_t value);
char *deserialize_uint32(char *buffer, uint32_t *value);
char *serialize_ack(char *buffer, uint8_t ack);
char *deserialize_ack(char *buffer, uint8_t *ack);
char *serialize_message(char *buffer, char *message);
char *deserialize_message(char *buffer, char *message);

uint8_t send_pkt(Socket s, Packet *p, struct sockaddr *dest, socklen_t slen);

uint8_t receive_pkt(Socket s, Packet *p, struct sockaddr *si_other, socklen_t *slen);



#endif
