/*
 * Simple udp server
 */
#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h> // sleep during x s
#include <signal.h> 

#include "server.h"
#include "structure.h"

#define SERVER "127.0.0.1"
#define BUFLEN 512  		//Max length of buffer
#define PORT 8888   		//The port on which to listen for incoming data
#define MaxMessage 512		//Maximum messages that we can receive simultaneously

#define DEBUG

typedef struct sockaddr_in Sockaddr_in;
typedef int Socket;





IDMessage Tab[MaxMessage];	// Array of temporarily stored messages

void bug(char *s)
{
	perror(s);
	exit(1);
}


// Execute this function after x seconds if you call alarm(x)

// void handle_alarm( int sig ) {
// 	printf("salut");
// }


/*
Handshake function for the server.
Wait for the message "initialization" and reply "initialization".
*/
int handshakeServer(Socket s) {
	Sockaddr_in si_other;
	char* message = "initialization";
	unsigned int recv_len, slen=10;  // Initialization at 10, do not understand why it does not work otherwise
	char buf[BUFLEN];
	memset(buf,'\0', BUFLEN);

	while (strcmp(buf, message)) {
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1) bug("recvfrom()");
	}
	printf("The server received the message : %s\n", buf);
	//printf("oo, %d, %d", slen, sizeof(si_other));
	
	//now reply the client with "initialization"
	if (sendto(s, message, sizeof(message), 0, (struct sockaddr*) &si_other, sizeof(si_other)) == -1) bug("sendto()");
	return 0;
}

/*
Handshake function for the client.
Send the message "initialization" and wait for the answer "initialization".
*/
int handshakeClient(Socket s, Sockaddr_in* si_other_p) {
	Sockaddr_in si_garbage;
	char* message = "initialization";
	unsigned int recv_len, slen;
	char buf[BUFLEN];
	memset(buf,'\0', BUFLEN);

	// Set a timeout to wait before resending a connection
	// struct timeval tv;
	// tv.tv_sec = 1;  // 1 s timeout
	// tv.tv_usec = 0;  // Not init'ing this can cause strange errors

	// if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval))) {
	// 	bug("setsockopt()");
	// }
	// signal(EAGAIN, handle_alarm );
	// alarm(1);

	while (strcmp(buf, message)) {
		if (sendto(s, message, strlen(message), 0, (struct sockaddr*) si_other_p, sizeof(*si_other_p)) == -1) bug("sendto()");
		printf("Trying to connect ... \n");

		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_garbage, &slen)) == -1) bug("recvfrom()");
	}
	printf("The server received the message : %s \nThe communication can now begin\n", buf);
	//printf("oo, %d, %d", slen, sizeof(si_other));
	
	//now reply the client with "initialization"

	return 0;
}




int main(int argc, char **argv)
{
	Sockaddr_in si_other;
	Socket s;
	unsigned int slen = sizeof(si_other) , recv_len;
	char buf[BUFLEN];
	char message[BUFLEN];


	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) bug("socket");

	if(argc <2){
		printf("Enter the number of the canal: 0 for server, 1 pour client");
		return -1;
	}
	//Si c'est un server
	if(!strcmp(argv[1],"0")){
		Sockaddr_in si_me;
		IDMessage* pointerMessage;
		char* pointerBuffer;

		// zero out the structure
		memset((char *) &si_me, 0, sizeof(si_me));

		si_me.sin_family = AF_INET;
		si_me.sin_port = htons(PORT);
		si_me.sin_addr.s_addr = htonl(INADDR_ANY);

		//bind socket to port
		if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) bug("bind");

		// Wait for the initialization message
		//handshakeServer(s);

		// Init the Tab
		InitTab();

		//keep listening for data
		while(1)
		{
			printf("Waiting for data...");
			fflush(stdout);

			//Il faudra ici, bien vider le buffer = écrire un '\0' au début
			//Sinon quand le message est plus petite que l'ancien, on voit encore l'ancien
			//try to receive some data, this is a blocking call
			if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1) bug("recvfrom()");

			pointerMessage = findMessage(((enTete*) buf)->numMessage, ((enTete*) buf)->maxSequence);
			pointerBuffer = (char*) (((enTete*) buf)->numSequence * sizeof(char));
			memcpy(pointerBuffer, buf, BUFLEN);
			addValue(&(pointerMessage->list), ((enTete*) buf)->numSequence);
			checkCompletionMessage(&(pointerMessage->list), pointerMessage->maxSequence);

			

			//print details of the client/peer and the data received
			printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
			printf("Data: %s\n" , buf);

			//now reply the client with the same data
			if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1) bug("sendto()");
			memset(buf,'\0', BUFLEN);
		}
		close(s);
	}
	// Si c'est un client
	else if(!strcmp(argv[1],"1")){

		memset((char *) &si_other, 0, sizeof(si_other));
		si_other.sin_family = AF_INET;
		si_other.sin_port = htons(PORT);

		if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
		{
			fprintf(stderr, "inet_aton() failed\n");
			exit(1);
		}
		// Init the connection by sending a message and waiting for an answer
		//handshakeClient(s, &si_other);

		pid_t receive_pid;
		int receive_status;
		receive_pid = fork();
		// Processus de reception des messages
		if(receive_pid == 0){

		}else{  // Processus d'envoi des messages
			while(1){
#ifdef DEBUG
				printf("Enter message : ");
				fgets(message, BUFLEN, stdin);

#endif
				printf("Le fils (%d) a lu : %s \n", getpid(), message);
				//send the message
				if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen)==-1) bug("sendto()");

				//receive a reply and print it
				//clear the buffer by filling null, it might have previously received data
				memset(buf,'\0', BUFLEN);
				//try to receive some data, this is a blocking call
				if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1) bug("recvfrom()");
				puts(buf);
			}
		}
		close(s);
		wait(&receive_status);
	}
	return 0;
}
