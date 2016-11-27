/*
:qa
 * ======================================================================
 *         Author:  Lucas MAHIEU (), lucas.mahieu@grenoble-inp.org
 * ======================================================================
 */

#include "window.h"
#include <sys/time.h>

void update_timeout(WaitAckElement* e, Time* t){
	gettimeofday(t,NULL);
	if (t->tv_usec + TIMEOUT_WAIT_ACK > MAX_TV_USEC) {
		e->timeout.tv_sec =  t->tv_sec + 1;
		e->timeout.tv_usec = (t->tv_usec + TIMEOUT_WAIT_ACK)%MAX_TV_USEC;
	} else {
		e->timeout.tv_sec = t->tv_sec;
		e->timeout.tv_usec = t->tv_usec + TIMEOUT_WAIT_ACK;
	}
}

// check if the packet can be accepted, return 1 if ok
uint8_t in_window(uint64_t last_number, uint64_t numPacket) {
	if (numPacket < (last_number + WINDOW_SIZE)) {
		if (numPacket >= last_number) {
			// perfect, in the window
			return 1;
		} else {
			// number too low, packet already received, resend ack
			return 0;
		}
	} else {
		// number too high, do nothing
		return 2;
	}
}

// ATTENTION: il faut checker si num_packet et dans la window avant !!
uint8_t update_Tab(uint64_t* last_number, uint64_t numPacket, uint64_t* Tab) {
	if (Tab[numPacket%WINDOW_SIZE] == numPacket) {
		// Le paquet a déjà été reçu. On ne doit pas le délivrer.
		return 1; 
	} else { // Sinon, on n'a jamais recu ce packet et on doit le délivrer .
		Tab[numPacket%WINDOW_SIZE] = numPacket;
		int i = 0;

		while (Tab[((*last_number)+i)%WINDOW_SIZE] == ((*last_number)+i)) {
			i++;
		}
		*last_number = *last_number + i;
		return 0;
	}
}
