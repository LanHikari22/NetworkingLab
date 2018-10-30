#include "receiver.h"
#include "clock.h"
#include "ringbuffer.h"
#include "tim.h"
#include "gpio.h"
#include "monitor.h"
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

// Forward reference
static void initInputCapture(enum TIMs);
static void initCounterTimer(enum TIMs);
static inline void stopTimeoutTimer();
static inline void startTimeoutTimer(uint32_t);

// initiates the receiver module
void receiver_init() {
	init_usart2(19200, F_CPU);
	// enable receive pin
	init_GPIO(RECEIVE_GPIO);
	enable_output_mode(RECEIVE_GPIO, RECEIVE_PIN);
	// configure Timers
	// TODO: the IO pin has to change based on TIM4
	init_GPIO(B);
	enable_af_mode(B, 6, 2);
	initInputCapture(TIM4);
	initCounterTimer(TIM3);
}

// Main routine update, this should execute inside a while(1); by what uses this module.
void receiver_mainRoutineUpdate() {
	if (monitorState == TS_IDLE) {
		// set for beginning of transmission, first bit automatically captured as zero
		if (!sample)
			sample = true;
		bool dataPresent = !isEmpty(receiveBuf);
		while (!isEmpty(&receiveBuf)) {
			printf(get(&receiveBuf));
		}
		if (dataPresent)
			printf('\n');
	}
}

// Input Capture Timer ISR used for receiving. Update if using a different timer
void TIM3_IRQHandler() {

	// case when we're in a half clock period edge
	if (sample) {
		// sample bit
		if (select_gpio(RECEIVE_GPIO)->IDR & (1<<RECEIVE_PIN) != 0) {
			dataByte |= (1<<currBit);
		}
		else {
			dataByte &= ~(1<<currBit);
		}

		currBit++;

		if (currBit == 8) {
			put(dataByte, &receiveBuf);
			currBit = 0;
		}
		// we should not sample next edge, unless timeout
		sample = false;

		// set timeout based on stamp of when edge ocurred
	}
	// case when we're in a clock period edge
	else {
		// the next edge is guaranteed to be a half clock period edge, where we always sample
		sample = true;

		// disable timeout, next edge occurs 100% of the time as per the manchester encoding

	}
}

// Counter Timer for Half bit timeout. Indicates whether to sample on the next half clock period or not
void TIM4_IRQHandler() {
	// if this timeout ocurrs, we're at bit period edge, the next must be a sample.
	sample = true;


}

// Enables the Input Capture Timer for stamping and sampling on edge. Sampling only occurs every half-period edge.
// assumes the gpio pin has already been set up to AF
static void initInputCapture(enum TIMs TIMER) {
	enable_timer_clk(TIMER);
	set_to_input_capture_mode(TIMER);
	set_to_counter_mode(TIMER);
	log_tim_interrupt(TIMER);
	enable_input_capture_mode_interrupt(TIMER);
	clear_cnt(TIMER);
	start_counter(TIMER);
}

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
