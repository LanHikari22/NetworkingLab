#include "uart_driver.h"
#include "tim.h"
#include "ringbuffer.h"
#include "receiver.h"
#include <inttypes.h>
#include <stdio.h>

// main
int main(void){
	// Initiate/start modules
	monitor_start();
//	transmitter_init();
	receiver_init();
//	init_usart2(19200, F_CPU);

	// Main routine
	while (1) {
//		transmitter_mainRoutineUpdate();
		receiver_mainRoutineUpdate();
	}
}
