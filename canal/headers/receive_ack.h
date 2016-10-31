/*
 * ======================================================================
 *         Author:  Lucas MAHIEU (), lucas.mahieu@grenoble-inp.org
 * ======================================================================
 */
#ifndef __RECEIVE_ACK__
#define __RECEIVE_ACK__

#include "structure.h"
#include "window.h"

typedef struct {
	WaitAckElement* windowTable; 
	uint32_t *iReSend;
	Sockaddr_in si_other;
	unsigned int slen;
	Socket s;

} ArgAck;


#endif
