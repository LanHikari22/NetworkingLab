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
	const bool EXTI9_ENABLE = false; // true if transmitter is used alone
	monitor_start(EXTI9_ENABLE);
	transmitter_init();
	receiver_init();

	// Main routine
	while (1) {
		transmitter_mainRoutineUpdate();
		receiver_mainRoutineUpdate();
	}
}
