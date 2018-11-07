#include "uart_driver.h"
#include "tim.h"
#include "ringbuffer.h"
#include "receiver.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>

static uint16_t encodeManchester(uint8_t data);

// main
int main(void){
	// Initiate/start modules
	init_usart2(19200, F_CPU);
	const bool EXTI9_ENABLE = true; // true if transmitter is used alone
	monitor_start(EXTI9_ENABLE);
	transmitter_init();
	receiver_init();

	// Main routine
	while (1) {
//		printf("Enter char:\n");
//		uint8_t c = usart2_getch();
//		printf("\nEncoded Manchester: \"%c\" -> \"%x\"\n", c, encodeManchester(c));

		transmitter_mainRoutineUpdate();
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
