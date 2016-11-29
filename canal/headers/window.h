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
#define TIMEOUT_WAIT_ACK   10000
#define MAX_TV_USEC       999999
#define WINDOW_SIZE 4901

typedef struct {
	Packet p;
	struct timeval timeout;
} WaitAckElement;

void update_timeout(struct timeval *e, struct timeval *t); 
uint8_t in_window(uint32_t fastOfUs,uint32_t numPacket);
uint8_t update_Tab(uint32_t* last_number, uint32_t numPacket, uint32_t* Tab);

#endif

