
/*
 * delay.c
 *
 *  Created on: March 14, 2018
 *      Author: sharana
 *      Description: Here we have methods that provide a
 *      certain amount of delay using the HPS timer.
 */

#include "delay.h"
#include <inttypes.h>
#include <stdio.h>

/*
 * The method provides a delay in seconds
 * and number of seconds are divided by the
 * input.
 * @param number
 */
void delay_s(uint32_t number){
	// chosing timer base
	volatile HPS_TIMER *hps_timer = (HPS_TIMER*) 0XFFC08000;

	hps_timer->CONTROL &= ~(1<<HPS_TIMER0_ENABLE);
	//number of times
	for (int i=0; i< number; i++)
	{
		//Load 100000000 int timer load.
		int count = 100000000;
		hps_timer->LOAD = count; //1s

		//Turn on counter.
		hps_timer->CONTROL |= (1<<HPS_TIMER0_MODE);
		hps_timer->CONTROL |= (1<<HPS_TIMER0_ENABLE);

		//Wait for flag go to 1.
		while (((hps_timer->INTERRUPT_STATUS)&(0b1))==0) {}

		//Turn off counter.
		hps_timer->CONTROL &= ~(1<<HPS_TIMER0_ENABLE);

	}
}

/*
 * The method provides a delay in milli-seconds
 * and number of milli-seconds are divided by the
 * input.
 * @param number
 */
void delay_ms(uint32_t number){
	HPS_TIMER *hps_timer = (HPS_TIMER*) 0XFFC08000;

		hps_timer->CONTROL &= ~(1<<HPS_TIMER0_ENABLE);
		//number of times
		for (int i=0; i< number; i++)
		{
			//Load 100000 int timer load.
			int count = 100000;
			hps_timer->LOAD = count; //1ms

			//Turn on counter.
			hps_timer->CONTROL |= (1<<HPS_TIMER0_MODE);
			hps_timer->CONTROL |= (1<<HPS_TIMER0_ENABLE);

			//Wait for flag go to 1.
			while (((hps_timer->INTERRUPT_STATUS)&(0b1))==0) {}

			//Turn off counter.
			hps_timer->CONTROL &= ~(1<<HPS_TIMER0_ENABLE);

	}
}

/*
 * The method provides a delay in nano-seconds
 * and number of nano-seconds are divided by the
 * input.
 * @param number
 */
void delay_ns(uint32_t number){
	HPS_TIMER *hps_timer = (HPS_TIMER*) 0XFFC08000;

		hps_timer->CONTROL &= ~(1<<HPS_TIMER0_ENABLE);
		//number of times
		for (int i=0; i< number; i++)
		{
			//Load 100 int timer load.
			int count = 100;
			hps_timer->LOAD = count; //1ns

			//Turn on counter.
			hps_timer->CONTROL |= (1<<HPS_TIMER0_MODE);
			hps_timer->CONTROL |= (1<<HPS_TIMER0_ENABLE);

			//Wait for flag go to 1.
			while (((hps_timer->INTERRUPT_STATUS)&(0b1))==0) {}

			//Turn off counter.
			hps_timer->CONTROL &= ~(1<<HPS_TIMER0_ENABLE);

	}
}
