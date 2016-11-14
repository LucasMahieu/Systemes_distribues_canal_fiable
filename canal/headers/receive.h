#ifndef __RECEIVE
#define __RECEIVE

#include "structure.h"

uint8_t in_window(uint64_t fastOfUs,uint64_t numPacket);
uint8_t update_Tab(uint64_t* last_number, uint64_t numPacket, uint64_t* Tab);
uint8_t check_end(Packet p);
#endif
