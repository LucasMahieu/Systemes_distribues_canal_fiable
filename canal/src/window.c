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
