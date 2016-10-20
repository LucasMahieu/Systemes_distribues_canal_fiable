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
	char buf[MAX_BUFLEN];
	char message[MAX_BUFLEN];							// Message to send / receive
	char packet[MAX_BUFLEN + sizeof(enTete)];			// En tête  + Message = packet

	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) bug("socket");

	if(argc <2){
		printf("Enter the number of the canal: 0 for server, 1 pour client\n");
		return -1;
	}
	
	//Si c'est un server : coté de B par convention
	if(!strcmp(argv[1],"0")){
		IDMessage* pointerIDMessage;
		char* pointerBuffer;

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
		InitTab();

		enTete E;
		char message[MAX_BUFLEN];

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
			if ((recv_len = recvfrom(s, buf, MAX_BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1) bug("recvfrom()");

			// Find (or allocate if first) the IDMessage struct in Tab where the message has to be stored.
			pointerIDMessage = findMessage(((enTete*) buf)->numMessage, ((enTete*) buf)->maxSequence); 
			// printTab();
			
			
			// Point on the begining of buffer in the IDMessage struct of Tab
			pointerBuffer = (char*) (pointerIDMessage->buffer + (((enTete*) buf)->numSequence-1)*MAX_BUFLEN);

			memcpy(pointerBuffer, buf + sizeof(enTete), MAX_BUFLEN);

			// printTab();
			addValue(&(pointerIDMessage->list), ((enTete*) buf)->numSequence);

			// printf("----------------------------------------------------------------------------------------");
			// printTab();
			// printf("----------------------------------------------------------------------------------------");


			//print details of the client/peer and the data received
			printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
			E.numMessage = 	((enTete*)buf)->numMessage;
			E.numSequence = ((enTete*)buf)->numSequence;
			E.maxSequence = ((enTete*)buf)->maxSequence;
			printf("En Tête : (%d, %d, %d)\n", E.numMessage, E.numSequence, E.maxSequence);
			printf("Data : %s\n", buf + sizeof(enTete));

			// Is the message full ?
			if (checkCompletionMessage(&(pointerIDMessage->list), pointerIDMessage->maxSequence)) {
				printf("NON, le message n'est pas complet\n");
			} else {
				printf("OUI, le message est complet\n");
				// freeTabElement(pointerIDMessage);
				// Do not forget to recompose the message and use the pipe to deliver it.
			}

			printTab();
			printf("-sdffWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW--------------------------");
			//now reply the client with the ENTETE only
			if (sendto(s, buf, sizeof(enTete), 0, (struct sockaddr*) &si_other, slen) == -1) bug("sendto()");
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
			
			// Fill entete
			enTete E;
			E.numMessage = 1;
			E.numSequence = 1;
			E.maxSequence = 2;

			while(1){
				// Processus A va faire send(m), et gets recoit m
#ifdef DEBUG
				printf("Enter message : ");
				fgets(message, MAX_BUFLEN, stdin);


				memcpy(packet, &E, sizeof(enTete));
				memcpy(packet + sizeof(enTete), message, MAX_BUFLEN);

				printf("Le message que l'on vient d'envoyer : %s", packet+sizeof(enTete));
				printf("L'en tête est : (%d, %d, %d)\n\n", ((enTete*)packet)->numMessage, ((enTete*)packet)->numSequence, ((enTete*)packet)->maxSequence);
#endif
				//send the message sur le canal
				if (sendto(s, packet, MAX_BUFLEN + sizeof(enTete), 0, (struct sockaddr *) &si_other, slen)==-1) bug("sendto()");
				E.numSequence ++;

				//clear the buffer by filling null, it might have previously received data
				memset(buf,'\0', MAX_BUFLEN);
			}
		//wait(&receive_status);
		}
		close(s);
	}
	return 0;
}
