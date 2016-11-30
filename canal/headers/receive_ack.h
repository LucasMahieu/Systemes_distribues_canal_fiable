/*
 * ======================================================================
 *         Author:  Lucas MAHIEU (), lucas.mahieu@grenoble-inp.org
 * ======================================================================
 */
#ifndef __RECEIVE_ACK__
#define __RECEIVE_ACK__

#include <pthread.h>

#include "structure.h"
#include "window.h"
#include "bug.h"

typedef struct {
	WaitAckElement* windowTable; 
	uint32_t *iReSend;
	Sockaddr_in si_other;
	unsigned int slen;
	Socket s;
	uint8_t fault_detector;
} ArgAck;

extern pthread_mutex_t mutex_iReSend;

void* receive_ack(void* arg);

#endif
