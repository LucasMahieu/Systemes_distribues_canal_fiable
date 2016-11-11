/*
 * ======================================================================
 *         Author:  Lucas MAHIEU (), lucas.mahieu@grenoble-inp.org
 * ======================================================================
 */

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <sys/time.h>
#include <structure.h>

// time to wait before to resend the packet in us
#define TIMEOUT_WAIT_ACK 10000
#define MAX_TV_USEC 999999
#define WINDOW_SIZE 5

typedef struct timeval Time;
typedef struct {
	Packet p;
	Time timeout;
} WaitAckElement;

void update_timeout(WaitAckElement* e, Time* t); 

#endif

