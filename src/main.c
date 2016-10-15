/*
 * Simple udp server
 */
#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //exit(0);
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER "127.0.0.1"
#define BUFLEN 512  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data

typedef struct sockaddr_in Sockaddr_in;
typedef int Socket;

void die(char *s)
{
	perror(s);
	exit(1);
}

int main(int argc, char **argv)
{
	if(argc <2){
		printf("Enter the number of the canal: 0 for server, 1 pour client");
		return -1;
	}
	//Si c'est un server
	if(!strcmp(argv[1],"0")){
		Sockaddr_in si_me, si_other;
		Socket s;
		int i, slen = sizeof(si_other) , recv_len;
		char buf[BUFLEN];

		//create a UDP socket
		if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		{
			die("socket");
		}

		// zero out the structure
		memset((char *) &si_me, 0, sizeof(si_me));

		si_me.sin_family = AF_INET;
		si_me.sin_port = htons(PORT);
		si_me.sin_addr.s_addr = htonl(INADDR_ANY);

		//bind socket to port
		if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
		{
			die("bind");
		}

		//keep listening for data
		while(1)
		{
			printf("Waiting for data...");
			fflush(stdout);

			//Il faudra ici, bien vider le buffer = écrire un '\0' au début
			//Sinon quand le message est plus petite que l'ancien, on voit encore l'ancien
			//try to receive some data, this is a blocking call
			if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
			{
				die("recvfrom()");
			}

			//print details of the client/peer and the data received
			printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
			printf("Data: %s\n" , buf);

			//now reply the client with the same data
			if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
			{
				die("sendto()");
			}
		}
		close(s);
	}
	// Si c'est un client
	else if(!strcmp(argv[1],"1")){
		Sockaddr_in si_other;
		Socket s;
		int i, slen=sizeof(si_other);
		char buf[BUFLEN];
		char message[BUFLEN];

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

		while(1)
		{
			printf("Enter message : ");
			gets(message);

			//send the message
			if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen)==-1)
			{
				die("sendto()");
			}

			//receive a reply and print it
			//clear the buffer by filling null, it might have previously received data
			memset(buf,'\0', BUFLEN);
			//try to receive some data, this is a blocking call
			if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
			{
				die("recvfrom()");
			}

			puts(buf);
		}
		close(s);
	}
	return 0;
}
