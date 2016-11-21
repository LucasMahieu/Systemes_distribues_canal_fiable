#ifndef STRUCTURE
#define STRUCTURE

#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
// to pull the pipe
#include <poll.h>
#include <pthread.h>

#include "structure.h"
#include "receive.h"
#include "receive_ack.h"
#include "window.h"
#define DEBUG
#define TEST

int test=0;

static pthread_mutex_t mutex_iReSend = PTHREAD_MUTEX_INITIALIZER;

void bug(char *s)
{
	fprintf(stderr,"%s",s);
	fflush(stderr);
}

// Thread de réception des ack du canal A
void* receive_ack(void* arg){
	WaitAckElement* pTable = (WaitAckElement*)(((ArgAck*)(arg))->windowTable);
	uint32_t* pReSend = (uint32_t*)(((ArgAck*)(arg))->iReSend);
	uint32_t iReSendCpy = 0;

	Packet p;
#ifdef DEBUG
	bug("### CANAL de A -- Thread de reception des ack\n");
#endif
	while(1){
		if (recvfrom(((ArgAck*)(arg))->s, &p, sizeof(uint64_t)+sizeof(uint32_t)+sizeof(uint8_t)+sizeof(uint32_t), 0, (struct sockaddr *) &(((ArgAck*)(arg))->si_other), &(((ArgAck*)(arg))->slen)) == -1) bug("ack recvfrom()");

#ifdef DEBUG
		fprintf(stderr, "ack n° %llu recu\n", p.numPacket);
#endif
		pTable[p.numPacket%WINDOW_SIZE].p.ack = 1;
		pthread_mutex_lock(&mutex_iReSend); // lock
		iReSendCpy = *pReSend;
		pthread_mutex_unlock(&mutex_iReSend); // unlock
		if(p.numPacket==iReSendCpy){
			while(pTable[iReSendCpy%WINDOW_SIZE].p.ack!=0){
				iReSendCpy++;
			}
			pthread_mutex_lock(&mutex_iReSend); // lock
			*pReSend = iReSendCpy;
			pthread_mutex_unlock(&mutex_iReSend); // unlock
		}
	}
	pthread_exit(NULL);
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

		// Sockaddr to receive data
		Sockaddr_in si_me;
		memset((char *) &si_me, 0, sizeof(si_me)); 		// zero out the structure
		si_me.sin_family = AF_INET;
		si_me.sin_port = htons(PORT);
		si_me.sin_addr.s_addr = htonl(INADDR_ANY);

		//bind socket to port
		if(bind(s, (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) bug("bind");

		// Wait for the initialization message
		// Does not work atm
		//handshakeServer(s);

		// Init the Tab to store messages before delivering them
		uint64_t Tab[WINDOW_SIZE];	// init tab 
		uint8_t check_in_window ;

		Packet p;
		uint64_t oldWaitingAck = 0;

		//keep listening for data
		while(1)
		{
			//Il faudra ici, bien vider le buffer = écrire un '\0' au début
			//Sinon quand le message est plus petite que l'ancien, on voit encore l'ancien
			//try to receive some data, this is a blocking call
			if ((recv_len = recvfrom(s, &p, sizeof(p), 0, (struct sockaddr *) &si_other, &slen)) == -1) bug("recvfrom()");

#ifdef DEBUG 
			fprintf(stderr, "### CANAL de B ########\n");
			fprintf(stderr, "L'en tête est : (%u %"PRIu64", %u)\n", p.source, p.numPacket, p.ack);
			fprintf(stderr, "oldWaitingAck = %"PRIu64", taille =  %d \n", oldWaitingAck, p.size);
			fprintf(stderr, "Le message reçu : %s\n", p.message);
#endif
			check_in_window = in_window(oldWaitingAck, p.numPacket);

			// number of the packet is in the right window
			if (check_in_window==1) { 
				// if the packet has not been received yet, deliver it to B
				// + update the Tab and the oldWaitingAck value
				if (update_Tab(&oldWaitingAck, p.numPacket, Tab)==0) { 	
					
					// Fonction DELIVER a B
					fprintf(stdout, "%s", p.message);
					fflush(stdout);

#ifdef DEBUG
					fprintf(stderr, "Message délivré\n");
#endif
				}

				// Format the packet to fit a ACK
				p.source = getpid();
				p.ack = 1;
				p.size = 0;

				//now reply the client with the ack
				if (sendto(s, &p, sizeof(uint32_t)+sizeof(uint64_t)+sizeof(uint32_t)+sizeof(uint8_t), 0, (struct sockaddr*) &si_other, slen) == -1) bug("sendto()");
#ifdef DEBUG
				fprintf(stderr, "Message n°%llu ack\n",p.numPacket);
#endif
			} else if (check_in_window==0) { // packet number too low, need to resend the ack to canalA
				if (sendto(s, &p,  sizeof(uint64_t)+sizeof(uint32_t)+sizeof(uint8_t)+sizeof(uint32_t), 0, (struct sockaddr*) &si_other, slen) == -1) bug("sendto()");
#ifdef DEBUG
				fprintf(stderr, "Message n°%llu RE ack\n",p.numPacket);
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

		memset((char *) &si_other, 0, sizeof(si_other));
		si_other.sin_family = AF_INET;
		si_other.sin_port = htons(PORT);

		if (inet_aton(SERVER , &si_other.sin_addr) == 0) bug("inet_aton() failed\n");
		
		// Pour la fiabilisation du canal
		WaitAckElement windowTable[WINDOW_SIZE]; 
		uint32_t iMemorize=0;
		uint32_t iSend=0;
		uint32_t iReSend=0;
		volatile uint32_t iReSendCpy = 0;

		Time currentTime;
		gettimeofday(&currentTime, NULL);

		// Init the connection by sending a message and waiting for an answer
		//handshakeClient(s, &si_other);

		// Création du thread qui receptionne les ack
		pthread_t receive_thread;
		ArgAck arg_thread;
		arg_thread.iReSend = &iReSend;
		arg_thread.windowTable = windowTable;
		arg_thread.si_other = si_other;
		arg_thread.slen = slen;
		arg_thread.s = s;
		if (pthread_create(&receive_thread, NULL, receive_ack, (void*)&arg_thread)) bug("pthread create failure: ");
		
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
		p.numPacket = currentIDPacket;				// Number of the message. The first message has a value of 1 for this attribute.
		p.ack = 0;							// is ack or not
		p.size=0;

		// Processus A va faire send(m), et gets recoit m
		while(!stop){

			// cpy iReSend dans iReSendCpy
			pthread_mutex_lock(&mutex_iReSend); // lock
			iReSendCpy = iReSend;
			pthread_mutex_unlock(&mutex_iReSend); // unlock


			// Fonction qui test si il y a des choses à lire dans le pipe
			if(poll(pfd,1,0)<1){
#ifdef DEBUG 
				//bug("NO DATA TO READ, WAITING FOR DATA IN CANAL A or Resending no ack packet\n");
#endif
			}else{
				// Si il y a assez de place pour le mémoriser 
				if(iMemorize<iReSendCpy+WINDOW_SIZE){
					if(fgets(message, MAX_BUFLEN, stdin) == NULL){
						if(ferror(stdin)) bug("## CANAL A: Erreur fgets\n");
						//bug("Canal A : EOF Received\n");
						//stop=1;
						continue;
					}

#ifdef DEBUG
					//bug("### Canal A\n");
					//fprintf(stderr, "-----------------------------------------------------------------------------\n");
					//fprintf(stderr,"Canal A a reçu : %s ", message);
					//fprintf(stderr, "-----------------------------------------------------------------------------\n");
					//fflush(stderr);
#endif
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
				}else{

				}
			}


			// Mtn il faux choisir si on envoie un message ou si on RE envoi un message non ack
			// On test si le plus vieux des msg non ack a dépassé son timeout
			if( iReSendCpy != iMemorize
				&&
				windowTable[iReSendCpy%WINDOW_SIZE].timeout.tv_sec <= currentTime.tv_sec 
				&& 
				windowTable[iReSendCpy%WINDOW_SIZE].timeout.tv_usec <= currentTime.tv_usec){

				// On prend le packet à envoyer
				toSendp = &(windowTable[iReSendCpy%WINDOW_SIZE].p);

				// On maj l'heure pour connaitre le nouveau timeout
				update_timeout(&(windowTable[iReSendCpy%WINDOW_SIZE]), &currentTime); 


//#ifdef TEST
//			if (toSendp->numPacket%11==0 && ((test%10)==9)) {
//				memset(toSendp->message,'\0', MAX_BUFLEN);
//				test +=1;
//				continue;
//			}
//			// fprintf(stderr, "Ca marche ou pas ????????????????? %"PRIu64"\n", p.numPacket);
//#endif


			// RE send the message sur le canal
			if (sendto(s, toSendp, sizeof(uint64_t)+sizeof(uint32_t)+sizeof(uint8_t)+sizeof(uint32_t)+sizeof(char)*(toSendp->size)+7, 0, (struct sockaddr *) &si_other, slen)==-1) bug("sendto()");
#ifdef DEBUG
			fprintf(stderr,"### CANAL A     ##################\n");
			fprintf(stderr,"L'en tête est : (%u, %"PRIu64", %u)\n", toSendp->source, toSendp->numPacket, toSendp->ack);
			fprintf(stderr,"Message RE envoyé: %s\n", toSendp->message);
			fprintf(stderr,"----------------------------------\n");
#endif
			}else if(iSend<iMemorize){
				// Si on a pas de msg qui ont dépassé le timeout, on envoie le plus vieux à envoyer
				toSendp = &(windowTable[iSend%WINDOW_SIZE].p);
				// On maj le timeout du packet à envoyé dans le cas ou l'on 
				// doit le re-envoyer plus tard
				update_timeout(&(windowTable[iSend%WINDOW_SIZE]), &currentTime); 
				iSend++;

//#ifdef TEST
//				if ((toSendp->numPacket%11)==10 && !((test%11)==10)) {
//				if ((toSendp->numPacket%10)==0) {
//					memset(toSendp->message,'\0', MAX_BUFLEN);
//					test +=1;
//					continue;
//				}
//				// fprintf(stderr, "Ca marche ou pas ????????????????? %"PRIu64"\n", p.numPacket);
//#endif

//send the message sur le canal
				if (sendto(s, toSendp, sizeof(uint64_t)+sizeof(uint32_t)+sizeof(uint8_t)+sizeof(uint32_t)+sizeof(char)*(toSendp->size)+7, 0, (struct sockaddr *) &si_other, slen)==-1) bug("sendto()");
#ifdef DEBUG
				fprintf(stderr,"### CANAL A     ##################\n");
				printf("L'en tête est : (%u, %"PRIu64", %u)\n", toSendp->source, toSendp->numPacket, toSendp->ack);
				printf("Message envoyé : %s\n", toSendp->message);
				fprintf(stderr,"----------------------------------\n\n");
#endif
			}
			//clear the buffer by filling null, it might have previously received data
		memset(p.message,'\0', MAX_BUFLEN);
#ifdef DEBUG
		fflush(stdout);
#endif
		}
		pthread_cancel(receive_thread);
		if(pthread_join(receive_thread,NULL)) bug("pthread join failure: ");
	}
	close(s);
	return 0;
}

#endif
