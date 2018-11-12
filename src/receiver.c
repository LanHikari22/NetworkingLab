#include "receiver.h"
#include "clock.h"
#include "ringbuffer.h"
#include "tim.h"
#include "gpio.h"
#include "monitor.h"
#include "uart_driver.h"
#include "monitor.h"
#include "packet_header.h"
#include "io_definitions.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>


// buffer to hold the received message
static RingBuffer receiveBuf = {0, 0};
// variable to hold each byte as it arrives
static int currBit;
static uint8_t dataByte = 0;
// flag that indicates the start of a message
static bool currentlyReceiving = false;
// flag to sample on the line per the input capture ISR
static bool sample = true;
// if true, only send packets.
static bool packetMode = false;
// Forward reference
static void initExternalInterrupt();
//static void initInputCapture(enum TIMs);
static void initCounterTimer(enum TIMs);
static inline void stopTimeoutTimer();
static inline void startTimeoutTimer(uint32_t);

// initiates the receiver module
void receiver_init(bool packet_mode) {
	init_usart2(19200, F_CPU);
	init_GPIO(C);
	// DEBUG: PC6 - Sample Toggle
	enable_output_mode(C, 6);
	// DEBUG: PC8 - Even bitperiod Counter Toggle
	enable_output_mode(C, 8);

	packetMode = packet_mode;

	// the receive pin has to change based on the specific timer.
	initExternalInterrupt();
	initCounterTimer(TIM4);
}

// Main routine update, this should execute inside a while(1); by what uses this module.
void receiver_mainRoutineUpdate() {
	// buffer to retrieve from the receive ringer buffer and a cursor into it
	static uint8_t msgBuf[sizeof(PacketHeader)];
	static int msgCur = 0;
	// the packet representation of the buffer
	static PacketHeader pkt;

	if (monitor_getState() == MS_IDLE) {
		// reset the transmission state, since we're IDLE. Next transmission is a new transmission
		currBit = dataByte = 0;
		currentlyReceiving = false;

		// set for beginning of transmission, first bit automatically captured as zero
		if (!sample)
			sample = true;
	}

	if (currentlyReceiving) {
		if (monitor_getState() == MS_COLLISION) {
			printf("<< COLLISION!\r\n");
			// cease all receiving
			stop_counter(TIM4);
			currentlyReceiving = false;
			// drop partially received message
			receiveBuf.put = receiveBuf.get = 0;
		}
	}

	if (!currentlyReceiving && hasElement(&receiveBuf)) {
		// grab the transmitted message, and display it
		msgCur = 0;
		while (hasElement(&receiveBuf) && msgCur < sizeof(PacketHeader)) {
			msgBuf[msgCur++] = get(&receiveBuf);
		}

		if (packetMode) {
			if (!ph_parse(&pkt, msgBuf, msgCur))
				printf("<< ERROR: invalid packet\r\n");

			if (pkt.length < PH_MSG_SIZE-1)
				pkt.msg[pkt.length] = '\0'; // this is to display the string, since the null terminator of a string literal is not part of the msg

			printf("< src=%x, dest=%x, length=%d, crc8=%x\r\n", pkt.src, pkt.dest, pkt.length, pkt.crc8_fcs);
			printf("< %s\r\n", pkt.msg);
		}
		else {
			if (msgCur < PH_MSG_SIZE-1)
				msgBuf[msgCur+1] = '\0'; // for proper display
			printf("< %s", msgBuf);
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
		if (!currentlyReceiving)
			currentlyReceiving = true;

		// DEBUG PC6: toggle to track sample ISR calls
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

	// DEBUG PC8: toggle to track ISR calls (halftime)
	GPIOC_BASE->ODR ^= 1<<8;

	// if this timeout occurs, we're at bit period edge, the next must be a sample.
	sample = true;

	// timeout occurred, shouldn't occur again unless a half bit period measures to a bit period.
	stop_counter(TIM4);
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

