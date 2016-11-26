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
#define WINDOW_SIZE 50

typedef struct timeval Time;
typedef struct {
	Packet p;
	Time timeout;
} WaitAckElement;

void update_timeout(WaitAckElement* e, Time* t); 
uint8_t in_window(uint64_t fastOfUs,uint64_t numPacket);
uint8_t update_Tab(uint64_t* last_number, uint64_t numPacket, uint64_t* Tab);

#endif

