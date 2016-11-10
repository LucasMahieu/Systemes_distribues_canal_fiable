#include "structure.h"
#include "window.h"

// check if the packet can be accepted, return 1 if ok
uint8_t in_window(uint64_t last_number, uint64_t numPacket) {
	if (last_number + WINDOW_SIZE > numPacket) {
		if (numPacket >= last_number) {
			return 0;	// perfect, in the window
		} else {
			return 1;	// number too low, packet already received, resend ack
		}
	} else {
		return 2;	// number too high, do nothing
	}
}

// attention, il faudrait que le premier packet soit le numero 1
uint8_t update_Tab(uint64_t* last_number, uint64_t numPacket, uint64_t* Tab) {
	static uint8_t zero_received = 0; // boolean for the zero case

	if (zero_received==0 && numPacket==0) {
		Tab[numPacket%WINDOW_SIZE] = numPacket;
		zero_received = 1;
		return 0;
	} else if (Tab[numPacket%WINDOW_SIZE]==numPacket) {
		return 1; // Le paquet a déjà été reçu.
	} else { // sinon
		Tab[numPacket%WINDOW_SIZE] = numPacket;
		//fprintf(stderr, " %d, %d, %d, %d \n", (int) *last_number, (int)numPacket, (int)((*last_number))%WINDOW_SIZE, (int) Tab[1] );
		int i = 0;


		while (Tab[(i+(*last_number))%WINDOW_SIZE]==*last_number+i) {
			*last_number = *last_number + 1;
			i ++;
		}
		return 0;
	}
}
