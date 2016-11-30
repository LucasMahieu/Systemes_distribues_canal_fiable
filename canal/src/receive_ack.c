/*
 * ======================================================================
 *         Author:  Lucas MAHIEU (), lucas.mahieu@grenoble-inp.org
 * ======================================================================
 */
#include "receive_ack.h"
#include "structure.h"

//#define DEBUG
#define DETECTOR


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
		if (receive_pkt(((ArgAck*)(arg))->s, &p, 
				(struct sockaddr*)&(((ArgAck*)(arg))->si_other),
				&(((ArgAck*)(arg))->slen)) == 0)
			bug("thread ack recvfrom()");

		
		//if (recvfrom(((ArgAck*)(arg))->s, &p, sizeof(uint32_t)+sizeof(uint8_t),
		//			0, (struct sockaddr *) &(((ArgAck*)(arg))->si_other), 
		//			&(((ArgAck*)(arg))->slen)) == -1) 

		// Enable the ack flag for the packet received
		pTable[(p.numPacket)%WINDOW_SIZE].p.ack = 1;

#ifdef DEBUG
		fprintf(stderr, ">>>> ack n° %u recu <<<<\n", p.numPacket);
#endif
		// get the current value of iReSend
		pthread_mutex_lock(&mutex_iReSend); // lock
		iReSendCpy = *pReSend;
		pthread_mutex_unlock(&mutex_iReSend); // unlock

		// If the ack received was for the oldest packet, update iReSend 
		if(p.numPacket == iReSendCpy){
			while (pTable[(iReSendCpy)%WINDOW_SIZE].p.ack == 1) {
				// Si cette case était à 1, on la met à zero pour le prochain passage
				pTable[iReSendCpy%WINDOW_SIZE].p.ack = 0;
				iReSendCpy++;
#ifdef DETECTOR
		fprintf(stdout, "%u\n", iReSendCpy);
		fflush(stdout);
		// fprintf(stderr, "%u\n", iReSendCpy);
		// fflush(stderr);
#endif
			}
#ifdef DEBUG
		// fprintf(stderr, "iReSend %d >>devient>> %d (+%d)\n", 
		// 		p.numPacket, iReSendCpy, iReSendCpy - p.numPacket);
#endif
			pthread_mutex_lock(&mutex_iReSend); // lock
			*pReSend = iReSendCpy;
			pthread_mutex_unlock(&mutex_iReSend); // unlock
		}
	}
	pthread_exit(NULL);
}

