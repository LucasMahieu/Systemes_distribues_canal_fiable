#include "structure.h"
#include "window.h"

// check if the packet can be accepted, return 1 if ok
uint8_t in_window(uint64_t last_number, uint64_t numPacket) {
	if (last_number + WINDOW_SIZE > numPacket) {
		if (numPacket >= last_number) {
			return 1;	// perfect, in the window
		} else {
			return 0;	// number too low, packet already received, resend ack
		}
	} else {
		return -1;	// number too high, do nothing
	}
}

// attention, il faudrait que le premier packet soit le numero 1
uint8_t update_Tab(uint64_t* last_number, uint64_t numPacket, uint64_t* Tab) {
	if (Tab[numPacket%WINDOW_SIZE]==0) { // si c'est le premier packet
		Tab[numPacket%WINDOW_SIZE] = numPacket;
		*last_number = numPacket;
		return 0;
	} else { // sinon
		int i = 1;
		Tab[numPacket%WINDOW_SIZE] = numPacket;
		while (Tab[(i+(*last_number))%WINDOW_SIZE]==*last_number+i) {
			*last_number ++;
		}
		return 0;
	}
}
