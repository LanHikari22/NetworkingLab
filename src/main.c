#include "uart_driver.h"
#include "tim.h"
#include "ringbuffer.h"
#include "receiver.h"
#include "packet_header.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>

// main
int main(void){
	// Initiate/start modules
	init_usart2(19200, F_CPU);
	ph_init();

	const bool EXTI9_ENABLE = false;
	const bool PACKET_MODE = true;
	const uint8_t SRC = 0xAA;
	const uint8_t DEST = 0xBB;

	monitor_start(EXTI9_ENABLE); // exti9_enable = true if transmitter is used alone
	transmitter_init(SRC, DEST, PACKET_MODE);
	receiver_init(PACKET_MODE);

	// Main routine
	while (1) {
		transmitter_mainRoutineUpdate();
		receiver_mainRoutineUpdate();
	}
}
