#ifndef STRUCTURE
#include "structure.h"
#define STRUCTURE
#endif

#include "server.h"

#define DEBUG

void bug(char *s)
{
	perror(s);
	exit(1);
}

int main(int argc, char **argv)
{
	Sockaddr_in si_other;
	Socket s;
	unsigned int slen = sizeof(si_other);				//slen to store the length of the address when we receive a packet,
	unsigned int recv_len;								//recv_len to get the number of char we received

	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) bug("socket");

	if(argc <2){
		printf("Enter the number of the canal: 0 for server, 1 pour client\n");
		return -1;
	}
	
	//Si c'est un server : coté de B par convention
	if(!strcmp(argv[1],"0")){

		// Sockaddr to recevie data
		Sockaddr_in si_me;
		memset((char *) &si_me, 0, sizeof(si_me)); 		// zero out the structure
		si_me.sin_family = AF_INET;
		si_me.sin_port = htons(PORT);
		si_me.sin_addr.s_addr = htonl(INADDR_ANY);

		//bind socket to port
		if(bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) bug("bind");

		// Wait for the initialization message
		// Does not work atm
		//handshakeServer(s);

		// Init the Tab to store messages before delivering them
		uint64_t Tab[WINDOW_SIZE];	// init tab 

		Packet p;
		uint64_t fastOfUs = 0;

		//keep listening for data
		while(1)
		{
#ifdef DEBUG
			printf("Waiting for data...\n");
			fflush(stdout);
#endif
			//Il faudra ici, bien vider le buffer = écrire un '\0' au début
			//Sinon quand le message est plus petite que l'ancien, on voit encore l'ancien
			//try to receive some data, this is a blocking call
			if ((recv_len = recvfrom(s, &p, sizeof(p), 0, (struct sockaddr *) &si_other, &slen)) == -1) bug("recvfrom()");

#ifdef DEBUG 
			printf("L'en tête est : (%u %u, %u)\n", p.source, p.numPacket, p.ack);
			printf("Le message que l'on vient d'envoyer : %s\n", p.message);
#endif


			if (in_window(fastOfUs, p.numPacket) {

						p.source = get_pid();
						p.ack = 1;

			

					//now reply the client with the ENTETE only
					if (sendto(s, &p,  sizeof(uint64_t)+sizeof(uint32_t)+sizeof(uint8_t), 0, (struct sockaddr*) &si_other, slen) == -1) bug("sendto()");
			
					// Fonction DELIVER a B
					puts(p.message);
					Tab[p.numPacket%WINDOW_SIZE] = 1;
					fastOfUs = 

					//now reply the client with the same data
					memset(p.message,'\0', MAX_BUFLEN);
				}
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
							printf("L'en tête est : (%u %u, %u)\n", p.source, p.numPacket, p.ack);
				printf("Le message que l'on vient d'envoyer : %s\n", p.message);
			char message[MAX_BUFLEN];
			// init packet
			Packet p;
		 	p.source = get_pid(); 	// processus source
			p.numPacket = 0;				// Number of the message. The first message has a value of 1 for this attribute.
			p.ack = 0;							// is ack or not

			while(1){
				// Processus A va faire send(m), et gets recoit m

				printf("Enter message : ");
				fgets(message, MAX_BUFLEN, stdin);

				// copy message in the packet
				memcpy(p.message, message, sizeof(char)*strlen(message));


#ifdef DEBUG
				printf("L'en tête est : (%u %u, %u)\n", p.source, p.numPacket, p.ack);
				printf("Le message que l'on vient d'envoyer : %s\n", p.message);
#endif
				//send the message sur le canal
				if (sendto(s, &p, sizeof(uint64_t)+sizeof(uint32_t)+sizeof(uint8_t)+sizeof(char)*strlen(p.message), 0, (struct sockaddr *) &si_other, slen)==-1) bug("sendto()");
				p.numPacket ++;

				//clear the buffer by filling null, it might have previously received data
				memset(p.message,'\0', MAX_BUFLEN);
			}
		wait(&receive_status);
		}
		close(s);
	}
	return 0;
}
