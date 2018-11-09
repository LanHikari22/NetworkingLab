#include "uart_driver.h"
#include "tim.h"
#include "ringbuffer.h"
#include "receiver.h"
#include "packet_header.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>

static uint16_t encodeManchester(uint8_t data);

// main
int main(void){
	// Initiate/start modules
	init_usart2(19200, F_CPU);
	ph_init();
	const bool EXTI9_ENABLE = false; // true if transmitter is used alone
	monitor_start(EXTI9_ENABLE);
	transmitter_init();
	receiver_init();

	int size = 0;
//	unsigned int *bufSize, uint8_t src, uint8_t dest, bool crc_flag, const void* msg, uint8_t size
//	void *buf = ph_createBuf(&size, 0xAA, 0xBB, 0x01, "A", 1);

	// Main routine
	while (1) {
		transmitter_mainRoutineUpdate();
		receiver_mainRoutineUpdate();
	}
}
