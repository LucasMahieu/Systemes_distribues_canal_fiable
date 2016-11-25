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

} ArgAck;

// This thread is use to protect iReSend indice from main and receive_ack thread
static pthread_mutex_t mutex_iReSend = PTHREAD_MUTEX_INITIALIZER;

void* receive_ack(void* arg);

#endif
