/*
 * delay.h
 *
 *  Created on: March 14, 2017
 *      Author: sharana
 */

#ifndef DELAY_H_
#define DELAY_H_
#include <inttypes.h>
//*** TIMER***	(taken from sample program given by Dr. Adam Livingstone)
typedef struct{
		 uint32_t LOAD;
		 uint32_t COUNTER;
		 uint32_t CONTROL;
		 uint32_t END_OF_INTERRUPT;
		 uint32_t INTERRUPT_STATUS;
	} HPS_TIMER;
#define HPS_TIMER0_ENABLE 0
#define HPS_TIMER0_MODE 1

// here we prototype the methods that cause a delay
void delay_s(uint32_t);	// delay per second.
void delay_ms(uint32_t);	// delay per milli-second.
void delay_ns(uint32_t);	// delay per nano-second.

#endif /* DELAY_H_ */
