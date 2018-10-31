#include "uart_driver.h"
#include "tim.h"
#include "ringbuffer.h"
#include "receiver.h"
#include <inttypes.h>
#include <stdio.h>

static uint16_t encodeManchester(uint8_t data);

// main
int main(void){
	// Initiate/start modules
	monitor_start();
//	transmitter_init();
	receiver_init();
	init_usart2(19200, F_CPU);


	// Main routine
	while (1) {
//		transmitter_mainRoutineUpdate();
		printf("Enter char:\n");
		uint8_t c = usart2_getch();
		printf("Encoded Manchester: \"%x\"\n", encodeManchester(c));

		receiver_mainRoutineUpdate();
	}
}

static uint16_t encodeManchester(uint8_t data){
	uint16_t output = 0;
	for (int i=0; i<8; i++) {
		int bit = (data & (1<<i)) >> i;
		output |= (1^bit) << (i*2);
		output |= (0^bit) << (i*2+1);
	}
	return output;
}
