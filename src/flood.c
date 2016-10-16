/*
 * test program to flood the server
 */
#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h> // sleep during x s
#include <signal.h> 



#define SERVER "127.0.0.1"
#define BUFLEN 512  		//Max length of buffer
#define BUFDELLEN 2048	 	//Maximum messages that we can receive simultaneously
#define PORT 8888   		//The port on which to listen for incoming data


typedef struct sockaddr_in Sockaddr_in;
typedef int Socket;

void die(char *s)
{
	perror(s);
	exit(1);
}




/*
Handshake function for the client.
Send the message "initialization" and wait for the answer "initialization".
*/
int handshakeClient(Socket s, Sockaddr_in* si_other_p) {
	Sockaddr_in si_garbage;
	char* message = "initialization";
	int recv_len, slen;
	char buf[BUFLEN];
	memset(buf,'\0', BUFLEN);

	// Set a timeout to wait before resending a connection
	// struct timeval tv;
	// tv.tv_sec = 1;  // 1 s timeout
	// tv.tv_usec = 0;  // Not init'ing this can cause strange errors

	// if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval))) {
	// 	die("setsockopt()");
	// }
	// signal(EAGAIN, handle_alarm );
	// alarm(1);

	while (strcmp(buf, message)) {
		if (sendto(s, message, strlen(message), 0, (struct sockaddr*) si_other_p, sizeof(*si_other_p)) == -1)
		{
			die("sendto()");
		}
		printf("Trying to connect ... \n");

		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_garbage, &slen)) == -1)
		{
			die("recvfrom()");
		}
		printf("lol");
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
	int i, slen=sizeof(si_other), counter=0;
	char buf[BUFLEN];
	char message [10];

	if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		die("socket");
	}

	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);

	if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	// Init the connection by sending a message and waiting for an answer
	handshakeClient(s, &si_other);

	while(1)
	{
		counter ++;
		printf("Counter : %d\n", counter);
		snprintf(message, 10, "%d", counter);

		//send the message
		if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen)==-1)
		{
			die("sendto()");
		}
	}
	close(s);
	return 0;
}
