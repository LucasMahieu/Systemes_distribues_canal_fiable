#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
// to poll the pipe
#include <poll.h>
#include <pthread.h>
// Canal headers
#include "structure.h"
#include "receive_ack.h"
#include "window.h"
#include "receive_msg.h"
#include "bug.h"

// Uncomment to enable debug traces
#define DEBUG

// #define DETECTOR

// Uncomment to enable the simulation of message lost
//#define TEST_NO_SEND
#ifdef TEST_NO_SEND
	#define MODULO_TEST_NO_ACK 2
#endif

// Uncomment to enable the simulation of ack lost
//#define TEST_NO_ACK
#ifdef TEST_NO_ACK
	#define MODULO_TEST_NO_ACK 2
#endif

int main(int argc, char **argv)
{
	Sockaddr_in si_other;
	Socket s;
	//slen to store the length of the address when we receive a packet,
	unsigned int slen = sizeof(si_other);
	//recv_len to get the number of char we received
	unsigned int recv_len;

	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) bug("socket");

	if(argc <2){
		printf("Enter the number of the canal: 0 for server, 1 pour client\n");
		return -1;
	}

	//Si c'est un server : coté de B par convention
	if(!strcmp(argv[1],"0")){

#ifdef DETECTOR
		char messageFromD[256];
#endif
		
		// Sockaddr to receive data
		Sockaddr_in si_me;
		memset((char *) &si_me, 0, sizeof(si_me));
		si_me.sin_family = AF_INET;
		si_me.sin_port = htons(PORT);
		si_me.sin_addr.s_addr = htonl(INADDR_ANY);

		//bind socket to port
		if(bind(s, (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) bug("bind not correct");

		// Init the Tab to store messages before delivering them
		uint64_t Tab[WINDOW_SIZE];
		// Important pour que le update sache que le packer zero n'est pas déjà recu
		Tab[0]=1;

		// Local vars
		uint8_t check_in_window ;
		Packet p;

		// This var contains the oldest packet that we are waiting the ack
		uint64_t oldWaitingAck = 0;

#ifdef TEST_NO_ACK
		uint8_t test_no_ack = 0;
#endif
		// ici, la fonction ne va faire que délivrer/ack les messages reçus
		while(1)
		{
			// try to receive some data, this is a blocking call
			if ((recv_len = recvfrom(s, &p, sizeof(p), 0,
				(struct sockaddr *) &si_other, &slen)) == -1)
					bug("recvfrom()");

#ifdef DEBUG 
			fprintf(stderr, "### CANAL de B ########\n");
			fprintf(stderr, "L'en tête est : (%u %"PRIu64", %u)\n",
					p.source, p.numPacket, p.ack);
			fprintf(stderr, "oldWaitingAck = %"PRIu64", taille =  %d \n",
					oldWaitingAck, p.size);
			fprintf(stderr, "Le message reçu : %s\n", p.message);
#endif
			// Est ce que l'on doit ack le message ?
			check_in_window = in_window(oldWaitingAck, p.numPacket);

			// YES : the pkt is in the right window
			if (check_in_window == 1) { 
				// if the packet has not been received yet, deliver it to B
				// + update the Tab and the oldWaitingAck value
				if (update_Tab(&oldWaitingAck, p.numPacket, Tab) == 0) {

					// Fonction DELIVER a B
					deliver(p.message);
#ifdef DETECTOR
					fprintf(stderr, "avant lecture dans proc\n");
					if(fgets(messageFromD, 256, stdin) == NULL) bug("Erreur de communication avec D");
					fprintf(stderr, "%s\n", messageFromD);
					fprintf(stderr, "apres lecture dans proc\n");
#endif				
#ifdef DEBUG
					fprintf(stderr, "Message délivré\n");
#endif
				}
				// Format the packet to fit a ACK
				p.source = getpid();
				p.ack = 1;
				p.size = 0;
#ifdef TEST_NO_ACK
				//now reply the client with the ack
				if(test_no_ack%MODULO_TEST_NO_ACK != 0){
#endif
					if (sendto(s, &p, sizeof(uint64_t)+sizeof(uint32_t)+
						sizeof(uint32_t)+sizeof(uint8_t), 0, 
						(struct sockaddr*) &si_other, slen) == -1) 
							bug("sendto()");
#ifdef DEBUG
					fprintf(stderr, "Message n°%llu ack\n", p.numPacket);
#endif
#ifdef TEST_NO_ACK
				}
				test_no_ack ++;
#endif
			} else if (check_in_window == 0) { 

				// Format the packet to fit a ACK
				p.source = getpid();
				p.ack = 1;
				p.size = 0;

				// packet number too low, need to resend the ack to canalA
				if (sendto(s, &p,  sizeof(uint64_t)+sizeof(uint32_t)+
					sizeof(uint32_t)+sizeof(uint8_t),0, 
					(struct sockaddr*) &si_other, slen) == -1) 
						bug("canal B RE send : sendto()");
#ifdef DEBUG
				fprintf(stderr, "Message n°%llu RE ack\n", p.numPacket);
#endif
			} else { 
				// packet nummber too high, do nothing
			}
			// Clear message of the packet
			memset(p.message,'\0', MAX_BUFLEN);
#ifdef DEBUG
			fprintf(stderr, "----------------------\n");
			fflush(stderr);
#endif
		}
		close(s);
	}
	// Si c'est un client : il sera du coté de A
	else if(!strcmp(argv[1],"1")){
#ifdef DEBUG
	bug("### CANAL de A -- Thread principal d'envoi de données\n");
#endif

		// Paramétrage pour création du socket
		memset((char *) &si_other, 0, sizeof(si_other));
		si_other.sin_family = AF_INET;
		si_other.sin_port = htons(PORT);

		if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
			bug("inet_aton() failed\n");
		
		// Pour la fiabilisation du canal
		WaitAckElement windowTable[WINDOW_SIZE];
		uint32_t iMemorize=0;
		uint32_t iSend=0;
		uint32_t iReSend=0;
		volatile uint32_t iReSendCpy = 0;

		Time currentTime;
		gettimeofday(&currentTime, NULL);

		// Création du thread qui receptionne les ack
		pthread_t receive_thread;
		ArgAck arg_thread;
		arg_thread.iReSend = &iReSend;
		arg_thread.windowTable = windowTable;
		arg_thread.si_other = si_other;
		arg_thread.slen = slen;
		arg_thread.s = s;
		if (pthread_create(&receive_thread, NULL, receive_ack, (void*)&arg_thread)) 
			bug("pthread create failure: ");
		
		// Processus d'envoi des messages
		// structure à donner à poll() pour savoir si il y a des data à lire 
		struct pollfd pfd[1];
		pfd[0].fd = fileno(stdin);
		pfd[0].events = POLLIN | POLLPRI;

		volatile uint8_t stop=0; 

		char message[MAX_BUFLEN];
		uint64_t currentIDPacket=0;
		// init packet
		Packet p;
		Packet *toSendp=NULL;
		p.source = getpid(); 	// processus source
		// Number of the message. The first message has a value of 1 for this attribute.
		p.numPacket = currentIDPacket;
		// is ack or not
		p.ack = 0;
		p.size=0;

		uint8_t eof_received = 0;

		// Processus A va faire send(m), et gets recoit m
		while(!stop){

			// cpy iReSend dans iReSendCpy
			pthread_mutex_lock(&mutex_iReSend); // lock
			iReSendCpy = iReSend;
			pthread_mutex_unlock(&mutex_iReSend); // unlock


			// Fonction qui test si il y a des choses à lire dans le pipe
			if (eof_received == 0 && poll(pfd,1,0) < 1) {
#ifdef DEBUG 
				//bug("NO DATA TO READ, WAITING FOR DATA IN CANAL A or Resending no ack packet\n");
#endif
			}else{
				// Si il y a assez de place pour le mémoriser 
				if(eof_received == 0 && iMemorize < iReSendCpy+WINDOW_SIZE){
					if(fgets(message, MAX_BUFLEN, stdin) == NULL){
						if(ferror(stdin)) 
							bug("## CANAL A: Erreur fgets\n");
						if(feof(stdin)){ 
							bug("Canal A : EOF Received\n");
							eof_received = 1;
						}
						//stop=1;
						//continue;
					}
#ifdef DEBUG
					//bug("### Canal A\n");
					//fprintf(stderr, "-----------------------------------\n");
					//fprintf(stderr,"Canal A a reçu : %s ", message);
					//fprintf(stderr, "-----------------------------------\n");
					//fflush(stderr);
#endif
					if(!eof_received){
						// Filling the packet with some information and the data
						p.size = strlen(message);
						p.ack = 0;
						memcpy(p.message, message, (p.size)*sizeof(char));
						p.numPacket = currentIDPacket;
						currentIDPacket++;
						// On mémorise le packet reçu pour l'envoyer plus tard
						windowTable[iMemorize%WINDOW_SIZE].p = p;
						// Mise à jour du temps
						update_timeout(&(windowTable[iMemorize%WINDOW_SIZE]), &currentTime); 
						iMemorize++;
#ifdef DEBUG
						//printf("Message reçu de A: '%s'\n", p.message);
#endif
					}
				}
			}
			gettimeofday(&currentTime,NULL);

#ifdef DEBUG
//			fprintf(stderr, "window.time = %ld,%d et current.time = %ld,%d \n", 
//			windowTable[iReSendCpy%WINDOW_SIZE].timeout.tv_sec, 
//			windowTable[iReSendCpy%WINDOW_SIZE].timeout.tv_usec,
//			currentTime.tv_sec, currentTime.tv_usec);
#endif
			// Choix entre envoyer ou RE envoyer des messages

			// On test si le plus vieux des msg non ack a dépassé son timeout
			if ( iReSendCpy != iMemorize
				&&
				windowTable[iReSendCpy%WINDOW_SIZE].timeout.tv_sec <= currentTime.tv_sec 
				&& 
				windowTable[iReSendCpy%WINDOW_SIZE].timeout.tv_usec <= currentTime.tv_usec)
			{
				// On prend le packet à envoyer
				toSendp = &(windowTable[iReSendCpy%WINDOW_SIZE].p);

				// On maj l'heure pour connaitre le nouveau timeout
				update_timeout(&(windowTable[iReSendCpy%WINDOW_SIZE]), &currentTime); 

				// RE send the message sur le canal
				if (sendto(s, toSendp, sizeof(uint64_t)+sizeof(uint32_t)+
					sizeof(uint8_t)+sizeof(uint32_t)+sizeof(char)*(toSendp->size)+7, 
					0, (struct sockaddr *) &si_other, slen)==-1) 
						bug("sendto()");
#ifdef DEBUG
				fprintf(stderr,"### CANAL A     ##################\n");
				fprintf(stderr,"L'en tête est : (%u, %"PRIu64", %u)\n", 
						toSendp->source, toSendp->numPacket, toSendp->ack);
				fprintf(stderr,"Message RE envoyé: %s\n", toSendp->message);
				fprintf(stderr,"----------------------------------\n");

				fprintf(stderr,"---------------------\n");
				fprintf(stderr,"iReSend = %u\n",iReSendCpy);
				fprintf(stderr,"iSend = %u\n", iSend);
				fprintf(stderr,"iMemorize = %u\n",iMemorize);
				fprintf(stderr,"---------------------\n");
#endif
			// Si on a pas de msg qui ont dépassé le timeout, 
			} else if (iSend < iMemorize) {
				// on envoie le plus vieux à envoyer
				toSendp = &(windowTable[iSend%WINDOW_SIZE].p);

				// On maj le timeout du packet à envoyer
				update_timeout( &(windowTable[iSend%WINDOW_SIZE]), &currentTime); 
				iSend++;

				// Send the pakcet on the canal
				if (sendto(s, toSendp, sizeof(uint64_t)+sizeof(uint32_t)+
					sizeof(uint8_t)+sizeof(uint32_t)+sizeof(char)*(toSendp->size)+7, 
					0, (struct sockaddr *) &si_other, slen)==-1) 
						bug("sendto()");
#ifdef DEBUG
				fprintf(stderr,"### CANAL A     ##################\n");
				fprintf(stderr, "L'en tête est : (%u, %"PRIu64", %u)\n", 
						toSendp->source, toSendp->numPacket, toSendp->ack);
				printf("Message envoyé : %s\n", toSendp->message);
				fprintf(stderr,"----------------------------------\n\n");
				fprintf(stderr,"---------------------\n");
				fprintf(stderr,"iReSend = %u\n",iReSendCpy);
				fprintf(stderr,"iSend = %u\n", iSend);
				fprintf(stderr,"iMemorize = %u\n",iMemorize);
				fprintf(stderr,"---------------------\n");
#endif
			}
			//clear the buffer 
		memset(p.message,'\0', MAX_BUFLEN);
#ifdef DEBUG
		fflush(stderr);
#endif
		}
		fprintf(stderr,"### CANAL A: TEST FINISHED");
		pthread_cancel(receive_thread);
		if(pthread_join(receive_thread,NULL)) bug("pthread join failure: ");
	}
	close(s);
	return 0;
}

