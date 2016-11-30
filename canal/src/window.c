/*
 * ======================================================================
 *         Author:  Lucas MAHIEU (), lucas.mahieu@grenoble-inp.org
 * ======================================================================
 */

#include "window.h"
#include <sys/time.h>
#include "bug.h"

void update_timeout(struct timeval *timeout, struct timeval *c)
{
	gettimeofday(c, NULL);
	if (c->tv_usec + TIMEOUT_WAIT_ACK > MAX_TV_USEC) {
		timeout->tv_sec =  c->tv_sec + 1;
		timeout->tv_usec = (c->tv_usec + TIMEOUT_WAIT_ACK)%MAX_TV_USEC;
	} else {
		timeout->tv_sec = c->tv_sec;
		timeout->tv_usec = c->tv_usec + TIMEOUT_WAIT_ACK;
	}
}

// check if the packet can be accepted, return 1 if ok
uint8_t in_window(uint32_t last_number, uint32_t numPacket) {
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
uint8_t update_Tab(uint32_t* last_number, uint32_t numPacket, uint32_t* Tab) {
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
