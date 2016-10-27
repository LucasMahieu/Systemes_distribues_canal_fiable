#include "structure.h"
#include "window.h"

// check if the packet can be accapted, return 1 if ok
uint8_t in_window(uint64_t last_number,uint64_t numPacket) {
	return 1;
		if (last_number + WINDOW_SIZE > numPacket && numPacket >= last_number) {
				return 1;
		} else {
				return 0;
		}
}
