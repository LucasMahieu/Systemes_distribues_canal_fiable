#ifndef STRUCTURE
#define STRUCTURE

#include	<stdint.h>
#include <inttypes.h>
#include	<unistd.h>
// to pull the pipe
#include	<poll.h>
#include	<pthread.h>

#include "structure.h"
#include "receive.h"
#include "receive_ack.h"
#include	"window.h"
#define DEBUG

// Je crois que le mutex est inutile 
static pthread_mutex_t mutex_iReSend = PTHREAD_MUTEX_INITIALIZER;

void bug(char *s)
{
	fprintf(stderr,"%s",s);
	fflush(stderr);
}

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
		fprintf(stderr, "ack n° %llu recu\n",p.numPacket);
#endif
		pTable[p.numPacket].p.ack = 1;
		pthread_mutex_lock(&mutex_iReSend); // lock
		iReSendCpy = *pReSend;
		pthread_mutex_unlock(&mutex_iReSend); // unlock
		if(p.numPacket==iReSendCpy){
			while(pTable[iReSendCpy].p.ack!=0){
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
		uint64_t oldWaitingAck = 0;

		//keep listening for data
		while(1)
		{
			//Il faudra ici, bien vider le buffer = écrire un '\0' au début
			//Sinon quand le message est plus petite que l'ancien, on voit encore l'ancien
			//try to receive some data, this is a blocking call
			if ((recv_len = recvfrom(s, &p, sizeof(p), 0, (struct sockaddr *) &si_other, &slen)) == -1) bug("recvfrom()");

#ifdef DEBUG 
			fprintf(stderr, "### CANAL de B\n");
			fprintf(stderr, "L'en tête est : (%u %"PRIu64", %u)\n", p.source, p.numPacket, p.ack);
			fprintf(stderr, "Le message que l'on vient d'envoyer : %s\n", p.message);
			fflush(stderr);
#endif
			if (in_window(oldWaitingAck, p.numPacket)) {
				p.source = getpid();
				p.ack = 1;
				p.size = 0;

				// Fonction DELIVER a B
				puts(p.message);
				fflush(stdout);


				//now reply the client with the ENTETE only
				//if (sendto(s, &p,  sizeof(uint64_t)+sizeof(uint32_t)+sizeof(uint8_t)+sizeof(uint32_t), 0, (struct sockaddr*) &si_other, slen) == -1) bug("sendto()");
#ifdef DEBUG
				//fprintf(stderr, "### CANAL B: ack n°%llu sent to A\n",p.numPacket);
#endif
				Tab[p.numPacket%WINDOW_SIZE] = 1;
				oldWaitingAck ++;

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
		
		// Pour la fiabilisation du canal
		WaitAckElement windowTable[WINDOW_SIZE]; 
		uint32_t iMemorize=0;
		uint32_t iSend=0;
		uint32_t iReSend=0;
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
		if (pthread_create(&receive_thread, NULL, receive_ack, (void*)&arg_thread)) bug("pthread cread failure: ");
		
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
			if(poll(pfd,1,0)<1){
#ifdef DEBUG 
				//bug("NO DATA TO READ, WAITING FOR DATA IN CANAL A or Resending no ack packet\n");
#endif
			}else{
				// On test si il y a assez de place pour le mémoriser 
				//
				if(iMemorize<iReSend+WINDOW_SIZE){
					if(fgets(message, MAX_BUFLEN, stdin) == NULL){
						if(ferror(stdin)) bug("## CANAL A: Erreur fgets\n");
						//bug("Canal 1 : EOF Received\n");
						//stop=1;
						continue;
					}
					// Filling the packet with some information and the data
					p.size = strlen(message);
					memcpy(p.message, message, sizeof(char)*p.size);
					p.numPacket = currentIDPacket;
					currentIDPacket++;
					// On mémorise le packet reçu pour l'envoyer plus tard
					windowTable[iMemorize%WINDOW_SIZE].p = p;
					gettimeofday(&currentTime,NULL);
					windowTable[iMemorize%WINDOW_SIZE].timeout.tv_sec =  currentTime.tv_sec;
					windowTable[iMemorize%WINDOW_SIZE].timeout.tv_usec =  currentTime.tv_sec + TIMEOUT_WAIT_ACK;
					iMemorize++;
#ifdef DEBUG
					printf("### CANAL A\n");
					printf("L'en tête est : (%u, %"PRIu64", %u)\n", p.source, p.numPacket, p.ack);
					printf("Le message que l'on viens de recevoir de A : %s\n", p.message);
					fflush(stdout);
#endif
				}else{
					pthread_mutex_unlock(&mutex_iReSend); // unlock
					gettimeofday(&currentTime,NULL);
				}
			}
			// Mtn il faux choisir si on envoie un message ou si on RE envoi un message non ack
			// On test si le plus vieux des msg non ack a dépassé son timeout
			if (windowTable[iReSend%WINDOW_SIZE].timeout.tv_sec < currentTime.tv_sec || windowTable[iReSend].timeout.tv_usec < currentTime.tv_usec){
				toSendp = &(windowTable[iReSend%WINDOW_SIZE].p);
				gettimeofday(&currentTime,NULL);
				windowTable[iReSend%WINDOW_SIZE].timeout.tv_sec =  currentTime.tv_sec;
				windowTable[iReSend%WINDOW_SIZE].timeout.tv_usec =  currentTime.tv_sec + TIMEOUT_WAIT_ACK;
				//send the message sur le canal
				if (sendto(s, toSendp, sizeof(uint64_t)+sizeof(uint32_t)+sizeof(uint8_t)+sizeof(uint32_t)+sizeof(char)*(toSendp->size), 0, (struct sockaddr *) &si_other, slen)==-1) bug("sendto()");
#ifdef DEBUG
				printf("### CANAL A\n");
				printf("L'en tête est : (%u, %"PRIu64", %u)\n", toSendp->source, toSendp->numPacket, toSendp->ack);
				printf("Message RE envoyé: %s\n", toSendp->message);
				fflush(stdout);
#endif
			}else if(iSend<iMemorize){
				// Si on a pas de msg qui ont dépassé le timeout, on envoie le plus vieux à envoyé
				toSendp = &(windowTable[iSend%WINDOW_SIZE].p);
				gettimeofday(&currentTime,NULL);
				windowTable[iSend%WINDOW_SIZE].timeout.tv_sec =  currentTime.tv_sec;
				windowTable[iSend%WINDOW_SIZE].timeout.tv_usec =  currentTime.tv_sec + TIMEOUT_WAIT_ACK;
				iSend++;
				//send the message sur le canal
				if (sendto(s, toSendp, sizeof(uint64_t)+sizeof(uint32_t)+sizeof(uint8_t)+sizeof(uint32_t)+sizeof(char)*(toSendp->size), 0, (struct sockaddr *) &si_other, slen)==-1) bug("sendto()");
#ifdef DEBUG
				printf("### CANAL A\n");
				printf("L'en tête est : (%u, %"PRIu64", %u)\n", toSendp->source, toSendp->numPacket, toSendp->ack);
				printf("Message envoyé : %s\n", toSendp->message);
				fflush(stdout);
#endif
			}
			//clear the buffer by filling null, it might have previously received data
			memset(p.message,'\0', MAX_BUFLEN);
		}
		pthread_cancel(receive_thread);
		if(pthread_join(receive_thread,NULL)) bug("pthread join failure: ");
	}
	close(s);
	return 0;
}

#endif
