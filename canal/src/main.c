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
//#define DEBUG

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

pthread_mutex_t mutex_iReSend;

int main(int argc, char **argv)
{
	// Mutex to protect iReSend from receive ack thread
	if (pthread_mutex_init(&mutex_iReSend, NULL) != 0)
		bug("mutex_iReSend init failed\n");

	Sockaddr_in si_other;
	Socket s;
	//slen to store the length of the address when we receive a packet,
	unsigned int slen = sizeof(si_other);

	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) bug("socket");

	if(argc <2){
		printf("Enter the number of the canal: 0 for server, 1 pour client\n");
		return -1;
	}
	
	/** 
	 * Coté server du canal
	 * Dans notre cas il reçoit les messages, et les délivres
	 */
	if(!strcmp(argv[1],"0")){

#ifdef DETECTOR
		char messageFromD[256];
		int messageToRead;
#endif
		// Sockaddr to receive data
		Sockaddr_in si_me;
		memset((char *) &si_me, 0, sizeof(si_me));
		si_me.sin_family = AF_INET;
		si_me.sin_port = htons(PORT);
		si_me.sin_addr.s_addr = htonl(INADDR_ANY);

		//bind socket to port
		if (bind(s, (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) 
			bug("bind not correct");

		// Init the Tab to store messages before delivering them
		uint32_t Tab[WINDOW_SIZE] = {0};
		// Important pour que le update sache que le packer zero n'est pas déjà recu
		Tab[0] = 1;

		// Local vars
		uint8_t check_in_window ;
		// Pour recevoir les packets avec la sérialisation
		Packet p;
		p.ack = 0;
		p.numPacket = 0;
		memset(p.message,'\0', MAX_MES_LEN);

		// This var contains the oldest packet that we are waiting the ack
		uint32_t oldWaitingAck = 0;

#ifdef TEST_NO_ACK
		uint8_t test_no_ack = 0;
#endif
		// ici, la fonction ne va faire que délivrer/ack les messages reçus
		while(1)
		{
			// Receive some data, this is a blocking call
			// le tableau de char toRec 
			if (receive_pkt(s, &p, (struct sockaddr*)&si_other, &slen) == 0)
					bug("Error receive_pkt");

			//if ((recv_len = recvfrom(s, &p, sizeof(p), 0,
			//	(struct sockaddr *) &si_other, &slen)) == -1)
			

#ifdef DEBUG 
			fprintf(stderr, "### CANAL de B ########\n");
			fprintf(stderr, "L'en tête est : (num=%u, ack=%u)\n",
					p.numPacket, p.ack);
			fprintf(stderr, "oldWaitingAck = %u\n", oldWaitingAck);
			//fprintf(stderr, "Le message reçu : %s\n", p.message);
#endif
			// Permet de savoir si on doit délivrer le message
			check_in_window = in_window(oldWaitingAck, p.numPacket);

			// OUI, le packet est dans la fenêtre
			if (check_in_window == 1) { 
				// if the packet has not been received yet, deliver it to B
				// + update the Tab and the oldWaitingAck value
				if (update_Tab(&oldWaitingAck, p.numPacket, Tab) == 0) {

					// Fonction DELIVER a B
					deliver(p.message);
#ifdef DETECTOR
	#ifdef DEBUG
					fprintf(stderr, "avant lecture dans proc\n");
	#endif				
					read(fileno(stdin), messageFromD, 256);
	#ifdef DEBUG
					// fprintf(stderr, "Message à envoyer (canal D) : %s\n", messageFromD);
					fprintf(stderr, "apres lecture dans proc\n");
	#endif				
#endif				
#ifdef DEBUG
					fprintf(stderr, "Message délivré\n");
#endif
				} else {
#ifdef DEBUG
					fprintf(stderr, "Message non délivré\n");
#endif
				}
				// Format the packet to fit a ACK
				p.ack = 1;
				p.message[0]='\0';
#ifdef TEST_NO_ACK
				//now reply the client with the ack
				if (test_no_ack%MODULO_TEST_NO_ACK != 0) {
#endif
					if (send_pkt(s, &p, (struct sockaddr*)&si_other, slen) == 0)
						bug("Error sendto() the ack");
					
#ifdef DEBUG
					fprintf(stderr, "Message n°%u ack\n", p.numPacket);
#endif
#ifdef TEST_NO_ACK
				}
				test_no_ack ++;
#endif
			} else if (check_in_window == 0) { 
				// Format the packet to fit a ACK
				p.ack = 1;
				p.message[0]='\0';

				// NON: Le packet a déjà été reçu, on le re ack
				if (send_pkt(s, &p, (struct sockaddr*)&si_other, slen) == 0)
					bug("canal B RE send : sendto()");

#ifdef DEBUG
				fprintf(stderr, "Message non délivré\n");
				fprintf(stderr, "Message n°%u RE ack\n", p.numPacket);
#endif
			} else { 
				// NON : le packet n'est pas dans la fenêtre
				// On ne fait rien, on recevra se message à nouveau plus tard 
				// quand sa sera le bon moment
#ifdef DEBUG
				fprintf(stderr, "Message non délivré\n");
				fprintf(stderr, "Message n°%u non ack\n", p.numPacket);
#endif
			}
			// Clear message of the packet
			//memset(p.message,'\0', MAX_MES_LEN);
#ifdef DEBUG
			fprintf(stderr, "----------------------\n");
			fflush(stderr);
#endif
		}
		close(s);
	}
	/** 
	 * Coté 'client' du canal
	 * Dans notre cas il envoie les messages à l'autre processus
	 * Et s'assure qu'il les délivre
	 */
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
		uint32_t iMemorize = 0;
		uint32_t iSend = 0;
		uint32_t iReSend = 0;
		volatile uint32_t iReSendCpy = 0;

		struct timeval currentTime;
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
		
		// structure à donner à poll() pour savoir si il y a des data à lire 
		struct pollfd pfd[1];
		pfd[0].fd = fileno(stdin);
		pfd[0].events = POLLIN | POLLPRI;

		volatile uint8_t stop=0; 

		char message[MAX_MES_LEN];
		
		// Initialisation d'un packet vide qui servira de 'contenant' du message
		Packet p;
		Packet *toSendp = NULL;
		// processus source
		// Number of the message. 
		p.numPacket = iMemorize;
		// is ack or not
		p.ack = 0;

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
				if((eof_received == 0) && (iMemorize < iReSendCpy + WINDOW_SIZE)){
					if(fgets(message, MAX_MES_LEN, stdin) == NULL){
						if(ferror(stdin)) 
							bug("## CANAL A: Erreur fgets\n");
						if(feof(stdin)){ 
							bug("Canal A : EOF Received\n");
							eof_received = 1;
						}
					}
#ifdef DEBUG
					//bug("### Canal A\n");
					//fprintf(stderr, "-----------------------------------\n");
					//fprintf(stderr,"Canal A a reçu : %s \n", message);
					fprintf(stderr,"Canal A reçoit un nouveau message\n");
					//fprintf(stderr, "-----------------------------------\n");
#endif
					if(eof_received == 0){
						// Filling the packet with some information and the data
						p.ack = 0;
						p.numPacket = iMemorize;
						memcpy(p.message, message, strlen(message));
						// On mémorise le packet reçu pour l'envoyer plus tard
						windowTable[iMemorize%WINDOW_SIZE].p = p;
						// Mise à jour du temps
						update_timeout(&(windowTable[iMemorize%WINDOW_SIZE].timeout), &currentTime); 
						iMemorize++;
					}
				} else if (eof_received == 1) {
					if (iReSendCpy == iMemorize) {
						// in that case, we are sure to have send all msg
						stop = 1;
						continue;
					}
				}
			}
			// update the current time
			gettimeofday(&currentTime,NULL);

#ifdef DEBUG
//			fprintf(stderr, "window.time = %ld,%d et current.time = %ld,%d \n", 
//			windowTable[iReSendCpy%WINDOW_SIZE].timeout.tv_sec, 
//			windowTable[iReSendCpy%WINDOW_SIZE].timeout.tv_usec,
//			currentTime.tv_sec, currentTime.tv_usec);
#endif
			// Choix entre envoyer ou RE envoyer des messages

			// On test si le plus vieux des msg non ack a dépassé son timeout
			if ( iReSendCpy < iMemorize
				 &&
				 iReSendCpy < iSend
				 &&
				 ( windowTable[iReSendCpy%WINDOW_SIZE].timeout.tv_sec < currentTime.tv_sec 
					|| 
					(   windowTable[iReSendCpy%WINDOW_SIZE].timeout.tv_sec == currentTime.tv_sec 
						&&
						windowTable[iReSendCpy%WINDOW_SIZE].timeout.tv_usec <= currentTime.tv_usec
					)
				 )
				)
			{
				// On prend le packet à envoyer
				toSendp = &(windowTable[iReSendCpy%WINDOW_SIZE].p);

				// On maj l'heure pour connaitre le nouveau timeout
				update_timeout(&(windowTable[iReSendCpy%WINDOW_SIZE].timeout), &currentTime); 

				// RE send the message sur le canal
				if (send_pkt(s, toSendp, (struct sockaddr*)&si_other, slen) == 0)
					bug("error sendto() d'un RE envoie");
#ifdef DEBUG
				fprintf(stderr,"\n### CANAL A     ##################\n");
				fprintf(stderr,"RE envoi de : (%u, %u)\n", toSendp->numPacket, 
						toSendp->ack);
				//fprintf(stderr,"Message RE envoyé: %s\n", toSendp->message);
				fprintf(stderr,"----------------------------------\n");
				fprintf(stderr,"iReSend = %u\n",iReSendCpy);
				fprintf(stderr,"iSend = %u\n", iSend);
				fprintf(stderr,"iMemorize = %u\n",iMemorize);
				fprintf(stderr,"----------------------------------\n");
#endif
			// Si on a pas de msg qui ont dépassé le timeout, 
			} else if (iSend < iMemorize) {
				// on envoie le plus vieux à envoyer
				toSendp = &(windowTable[iSend%WINDOW_SIZE].p);

				// On maj le timeout du packet à envoyer
				update_timeout(&(windowTable[iSend%WINDOW_SIZE].timeout), &currentTime); 
				iSend++;

				// Send the pakcet on the canal
				if (send_pkt(s, toSendp, (struct sockaddr*)&si_other, slen) == 0)
					bug("error sendto() d'un envoie");
#ifdef DEBUG
				fprintf(stderr,"\n### CANAL A     ##################\n");
				fprintf(stderr,"Envoi de : (%u, %u)\n", toSendp->numPacket,
						toSendp->ack);
				//fprintf(stderr, "Message envoyé : %s\n", toSendp->message);
				fprintf(stderr,"----------------------------------\n");
				fprintf(stderr,"iReSend = %u\n",iReSendCpy);
				fprintf(stderr,"iSend = %u\n", iSend);
				fprintf(stderr,"iMemorize = %u\n",iMemorize);
				fprintf(stderr,"----------------------------------\n");
#endif
			}
			//clear the buffer 
			memset(p.message,'\0', MAX_MES_LEN);

		}
		fprintf(stderr,"### CANAL A: TEST FINISHED");
		pthread_cancel(receive_thread);
		if(pthread_join(receive_thread,NULL)) bug("pthread join failure: ");
	}
	close(s);
	return 0;
}

