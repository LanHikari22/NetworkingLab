#include "receiver.h"
#include "clock.h"
#include "ringbuffer.h"
#include "tim.h"
#include "gpio.h"
#include "monitor.h"
#include "uart_driver.h"
#include "monitor.h"
#include "io_definitions.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>


// buffer to hold the received message
static RingBuffer receiveBuf = {0, 0};
// variable to hold each byte as it arrives
static int currBit;
static uint8_t dataByte = 0;
// flag to sample on the line per the input capture ISR
static bool sample = true;
// flag that indicates the start of a message
static bool inTransmission = false;
// Forward reference
static void initExternalInterrupt();
//static void initInputCapture(enum TIMs);
static void initCounterTimer(enum TIMs);
static inline void stopTimeoutTimer();
static inline void startTimeoutTimer(uint32_t);

// initiates the receiver module
void receiver_init() {
	init_usart2(19200, F_CPU);
	init_GPIO(C);
	// TODO Debug PC6 - Sample Toggle
	enable_output_mode(C, 6);
	// TODO Debug PC8 - Even bitperiod Counter Toggle
	enable_output_mode(C, 8);

	// the receive pin has to change based on the specific timer. PC6 AF 2 is for TIM3_CH1
//	init_GPIO(C);
//	enable_af_mode(C, 6, 2);
//	initInputCapture(TIM3);
	initExternalInterrupt();
	initCounterTimer(TIM4);
}

// Main routine update, this should execute inside a while(1); by what uses this module.
void receiver_mainRoutineUpdate() {
	if (monitor_IDLE()) {
		// reset the transmission state, since we're IDLE. Next transmission is a new transmission
		currBit = dataByte = 0;
		inTransmission = false;

		// set for beginning of transmission, first bit automatically captured as zero
		if (!sample)
			sample = true;

	}

	if (!inTransmission) {

		// grab the transmitted message, and display it
		if (hasElement(&receiveBuf)) {
			printf("[echo] ");
			while (hasElement(&receiveBuf)) {
				printf("%c", get(&receiveBuf));
			}
			printf("\n");
		}
	}
}

void EXTI4_IRQHandler() {

	// Verify Interrupt is from EXTI4
	if (!((*(EXTI_PR)&(1<<4)) != 0))
		return;

	// Clear Interrupt
	*(EXTI_PR) |= 1<<4;

	// monitor the state of transmission
	monitor_Edge_Intrr();

	// case when we're in a half clock period edge
	if (sample) {
		// set timeout based on stamp of when edge occurred
		clear_cnt(TIM4);
		start_counter(TIM4);

		// we should not sample next edge, unless timeout
		sample = false;

		// sample bit
		if ( !!(GPIOC_BASE->IDR & (1<<4))) {
			dataByte |= (1<< (7-currBit));
		}
		else {
			dataByte &= ~(1<< (7-currBit));
		}

		currBit++;

		if (currBit == 8) {
			put(&receiveBuf, dataByte);
			currBit = 0;
		}

		// If this is the very first bit, indicate the start of a transmission
		if (!inTransmission)
			inTransmission = true;

		// TODO: debug, toggle PC6 to track ISR calls
		GPIOC_BASE->ODR ^= 1<<6;
	}
	// case when we're in a clock period edge
	else {
		// the next edge is guaranteed to be a half clock period edge, where we always sample
		sample = true;

		// disable timeout, next edge occurs 100% of the time as per the manchester encoding
		// TODO: use input capture, set timeout based on stamp of when edge occurred
		stop_counter(TIM4);

	}
}



// Counter Timer for Half bit timeout. Indicates whether to sample on the next half clock period or not
void TIM4_IRQHandler() {
	clear_output_cmp_mode_pending_flag(TIM4);

	// TODO: debug, toggle PC6 to track ISR calls
	GPIOC_BASE->ODR ^= 1<<8;

	// if this timeout occurs, we're at bit period edge, the next must be a sample.
	sample = true;

	// timeout occurred, shouldn't occur again unless a half bit period measures to a bit period.
	stop_counter(TIM4);
}

// Input Capture Timer ISR used for receiving. Update if using a different timer
//void TIM3_IRQHandler() {
//
//}


// Enables the Input Capture Timer for stamping and sampling on edge. Sampling only occurs every half-period edge.
// assumes the gpio pin has already been set up to AF
//static void initInputCapture(enum TIMs TIMER) {
//	enable_timer_clk(TIMER);
//	set_to_input_capture_mode(TIMER);
////	set_to_counter_mode(TIMER);
//	log_tim_interrupt(TIMER);
//	enable_input_capture_mode_interrupt(TIMER);
//	clear_cnt(TIMER);
//	start_counter(TIMER);
//}

// initiates the counter timer based on the HALFBIT_TIMEOUT_TICKS
static void initCounterTimer(enum TIMs TIMER) {
	enable_timer_clk(TIMER);
	set_arr(TIMER, HALFBIT_TIMEOUT_TICKS);
	set_ccr1(TIMER, HALFBIT_TIMEOUT_TICKS);
	// enables toggle on CCR1
	set_to_output_cmp_mode(TIMER);
	// enables output in CCER
	enable_output_output_cmp_mode(TIMER);
	clear_cnt(TIMER);
	start_counter(TIMER);
	enable_output_cmp_mode_interrupt(TIMER);
	// register and enable within the NVIC
	log_tim_interrupt(TIMER);
}

static void initExternalInterrupt(){
	// Enable Clock to SysCFG
	*(RCC_APB2ENR) |= 1<<14;
	// Enable Clock for PC4
	init_GPIO(C);
	enable_input_mode(C, 4);

	// Connect Syscfg to EXTI4, C-line
	*(SYSCFG_EXTICR2) &= ~(0b1111<<0);
	*(SYSCFG_EXTICR2) |= (0b0010<<0);

	// Unmask EXTI4 in EXTI
	*(EXTI_IMR) |= 1<<4;

	// Set falling edge
	*(EXTI_FTSR) |= 1<<4;

	// Set to rising edge
	*(EXTI_RTSR) |= 1<<4;

	// Set priority to max (IP[2*4+2] = IP2[24:], where each field is 8-bits, and NVIC idx 10 is field 2
	*(NVIC_IPR2) |= 0xF0 << 24;

	// Enable Interrupt in NVIC (Vector table interrupt enable)
	*(NVIC_ISER0) |= 1<<10;
}

