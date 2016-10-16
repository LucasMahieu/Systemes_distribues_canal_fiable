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

#define SERVER "127.0.0.1"
#define BUFLEN 512  		//Max length of buffer
#define BUFDELLEN 2048	 	//Max length buffer for storing messages (and wait for them to complete before delivering them)
#define PORT 8888   		//The port on which to listen for incoming data

#define MAX_BUFLEN 2048

//#define DEBUG

typedef struct sockaddr_in Sockaddr_in;
typedef int Socket;

void bug(char *s)
{
	perror(s);
	exit(1);
}
/*
Handshake function for the server.
Wait for the message "initialization" and reply "initialization".
*/
int handshakeServer(Socket s) {
	Sockaddr_in si_other;
	char* message = "initialization";
	unsigned int recv_len, slen=10;  // Initialization at 10, do not understand why it does not work otherwise
	char buf[MAX_BUFLEN];
	memset(buf,'\0', MAX_BUFLEN);

	while (strcmp(buf, message)) {
		if ((recv_len = recvfrom(s, buf, MAX_BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1) bug("recvfrom()");
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
	char buf[MAX_BUFLEN];
	memset(buf,'\0', MAX_BUFLEN);

	while (strcmp(buf, message)) {
		if (sendto(s, message, strlen(message), 0, (struct sockaddr*) si_other_p, sizeof(*si_other_p)) == -1) bug("sendto()");
		printf("Trying to connect ... \n");
		// sleep(1);		//sleep function does not work

		if ((recv_len = recvfrom(s, buf, MAX_BUFLEN, 0, (struct sockaddr *) &si_garbage, &slen)) == -1) bug("recvfrom()");
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
	char buf[MAX_BUFLEN];
	char message[MAX_BUFLEN];


	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) bug("socket");

	if(argc <2){
		printf("Enter the number of the canal: 0 for server, 1 pour client");
		return -1;
	}
	
	//Si c'est un server : coté de B par convention
	if(!strcmp(argv[1],"0")){
		Sockaddr_in si_me;
		//char delBuf[BUFDELLEN]; // Buffer for storing all the messages before delivering them

		// zero out the structure
		memset((char *) &si_me, 0, sizeof(si_me));

		si_me.sin_family = AF_INET;
		si_me.sin_port = htons(PORT);
		si_me.sin_addr.s_addr = htonl(INADDR_ANY);

		//bind socket to port
		if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) bug("bind");

		// Wait for the initialization message
		//handshakeServer(s);

		//keep listening for data
		while(1)
		{
#ifdef DEBUG			
			printf("Waiting for data...");
			fflush(stdout);
#endif
			//Il faudra ici, bien vider le buffer = écrire un '\0' au début
			//Sinon quand le message est plus petite que l'ancien, on voit encore l'ancien
			//try to receive some data, this is a blocking call
			if ((recv_len = recvfrom(s, buf, MAX_BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1) bug("recvfrom()");

#ifdef DEBUG
			printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
			printf("Data: %s\n" , buf);
#endif 

			// Fonction DELIVER
			puts(buf);

			//now reply the client with the same data
			//if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1) bug("sendto()");
			memset(buf,'\0', MAX_BUFLEN);
		}
		close(s);
	}
	// Si c'est un client : il sera du coté de A
	else if(!strcmp(argv[1],"1")){

		memset((char *) &si_other, 0, sizeof(si_other));
		si_other.sin_family = AF_INET;
		si_other.sin_port = htons(PORT);

		if (inet_aton(SERVER , &si_other.sin_addr) == 0) bug("inet_aton() failed\n");

		// Init the connection by sending a message and waiting for an answer
		//handshakeClient(s, &si_other);

		pid_t receive_pid=1;
		//int receive_status;
		//receive_pid = fork();
		// Processus de reception des messages
		if(receive_pid == 0){
			//try to receive some data, this is a blocking call
			//if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1) bug("recvfrom()");
			//puts(buf);
		}else{  // Processus d'envoi des messages
			while(1){
				// Processus A va faire send(m), et gets recoi m
				fgets(message, MAX_BUFLEN, stdin);
#ifdef DEBUG
				printf("Réception d'un msg venant de A par le canal de A : %s \n",message);
#endif
				//send the message sur le canal
				if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen)==-1) bug("sendto()");

				//clear the buffer by filling null, it might have previously received data
				memset(buf,'\0', MAX_BUFLEN);
			}
		//wait(&receive_status);
		}
		close(s);
	}
	return 0;
}
