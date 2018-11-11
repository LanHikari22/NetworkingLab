#include "transmitter.h"
#include "clock.h"
#include "ringbuffer.h"
#include "tim.h"
#include "gpio.h"
#include "monitor.h"
#include "packet_header.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// transmission buffer to put the manchester encoded bytes in for transmission
static RingBuffer transBuf = {0, 0};
// used by the timer ISR, determines which bit to send at the time of execution
static int currBit = 0;
static int currByte = 0;
// flags to tell whether a transmission is going, and whether the last one was complete. (no COLLISION)
static bool inTransmission = false;
static bool transmissionComplete = true;
static bool retransmissionTimeoutActive = false;

// Forward references
static void transmitByte(uint8_t);
static void transmitMessage(const PacketHeader *pkt);
static uint16_t encodeManchester(uint8_t);
static void initTransmissionTimer();
static void toggleRetransmission(bool retransmission);
static void startTransmission();
static void stopTransmission();

void transmitter_init() {
	init_usart2(19200, F_CPU);
	initTransmissionTimer();

	// Enable transmission pin
	init_GPIO(TRANSMISSION_GPIO);
	enable_output_mode(TRANSMISSION_GPIO, TRANSMISSION_PIN);

	// Init rng, used for retransmission
	srand(clock(0));
	// DEBUG: PC6 - Retransmission Timeout Period
	enable_output_mode(C, 6);
	// DEBUG: PC8 - Sync Signal
	init_GPIO(C);
	enable_output_mode(C, 8);
}

void transmitter_mainRoutineUpdate() {
	// only transmit a fully received message from the uart
	static bool gotMessage = false;
	// buffer to create packet header out of uart msg
	static PacketHeader pkt = {0};
	// buffer of the data sent through UART
	static uint8_t dataBuf[PH_MSG_SIZE];
	// cursor that makes sure not to retrieve more than PH_MSG_SIZE bytes into dataBuf
	static int dataCur = 0;

	if (inTransmission && transmissionComplete) {
		if (currByte > 2*7 + 2*5) {

			monitor_jam();
		}
	}
	else if (!inTransmission && transmissionComplete) {
		char c = usart2_getch();

		// data to transmit received, transmit it
		if (c == '\r') {
			dataBuf[dataCur] = '\0';

			// just pressed enter. Don't care about that message
			if (!strcmp((char*)dataBuf, "\0e\0p")) {
				dataBuf[0] = dataCur = 0;
				gotMessage = false;
			}
			else {
				gotMessage = true;
			}
		}
		// read in data until new line or max buffer size reached
		else if (dataCur < PH_MSG_SIZE-1) {
			dataBuf[dataCur++] = c;
		}

		// start transmission when in TS_IDLE, and there's something to transmit
		if (monitor_getState() == MS_IDLE && !inTransmission && gotMessage) {
			currByte = 0;
			gotMessage = false;
			// transmit all characters
			transBuf.put = transBuf.get = 0;
			ph_create(&pkt, 0xAA, 0xBB, true, &dataBuf, dataCur);
			if (pkt.length < PH_MSG_SIZE-1)
				pkt.msg[pkt.length] = '\0'; // this is to display the string, since the null terminator of a string literal is not part of the msg
			printf("> src=%x, dest=%x, length=%d, crc8=%x\r\n", pkt.src, pkt.dest, pkt.length, pkt.crc8_fcs);
			printf("> %s\r\n", pkt.msg);
			// clear message
			dataBuf[0] = dataCur = 0;
			// setup for transmission
			transmitMessage(&pkt);
			startTransmission();
		}
	}
	// retransmit after a timeout period
	else if (monitor_getState() == MS_IDLE && !inTransmission && !transmissionComplete){
		currByte = currBit = 0; // transmit same message from the start
		toggleRetransmission(true);
		startTransmission();
	}
}

/**
 * Initiates the timer for the transmission on the PC9 line
 */
static void initTransmissionTimer() {
	enable_timer_clk(TRANSMITTER_TIMER);
	set_arr(TRANSMITTER_TIMER, TRANSMISSION_TICKS);
	set_ccr1(TRANSMITTER_TIMER, TRANSMISSION_TICKS);
	set_psc(TRANSMITTER_TIMER, 0);
	// enables toggle on CCR1
	set_to_output_cmp_mode(TRANSMITTER_TIMER);
	// enables output in CCER
	enable_output_output_cmp_mode(TRANSMITTER_TIMER);
	clear_cnt(TRANSMITTER_TIMER);
	start_counter(TRANSMITTER_TIMER);
	enable_output_cmp_mode_interrupt(TRANSMITTER_TIMER);
	// register and enable within the NVIC
	log_tim_interrupt(TRANSMITTER_TIMER);
}

/**
 * toggles the function of the timer from transmission to retransission timeout
 * or vice versa. This is to use the same timer for the random timeout period.
 * if set to retransmission mode, this computes a random time and sets the ticks to that
 * and starts the timer.
 */
static void toggleRetransmission(bool retransmission) {
	retransmissionTimeoutActive = retransmission;
	stop_counter(TRANSMITTER_TIMER);
	if (retransmission) {
		// configure timer for retransmission timeout
		set_psc(TRANSMITTER_TIMER, 16000); // ms scale
		// generate exact timeout
		int N = rand() % TRANSMITTER_N_MAX;
		int w = N *1000/TRANSMITTER_N_MAX;
		printf(">> Retransmitting in %d ms...\r\n", w);
		// set count
		set_arr(TRANSMITTER_TIMER, w);
		set_ccr1(TRANSMITTER_TIMER, w);
	}
	else {
		set_psc(TRANSMITTER_TIMER, 0); // cpu scale
		set_arr(TRANSMITTER_TIMER, TRANSMISSION_TICKS);
		set_ccr1(TRANSMITTER_TIMER, TRANSMISSION_TICKS);
	}

}

/**
 * This resets itself to transmit as long as the transmission buffer is not empty.
 * It changes the PC9 line every half-clock period in order to match the required bit rate.
 */
void TIM2_IRQHandler(){
	clear_output_cmp_mode_pending_flag(TRANSMITTER_TIMER);

	// retransmission mode, start transmission after timeout
	if (retransmissionTimeoutActive) {
		toggleRetransmission(false);
		startTransmission();
		// TODO DEBUG: to confirm timeout period
//		clear_cnt(TRANSMITTER_TIMER);
//		select_gpio(C)->ODR ^= (1<<6);
		return;
	}
	// TODO DEBUG
	select_gpio(C)->ODR ^= (1<<6);

	// Transmission complete, nothing else to transmit
	if (!hasElement(&transBuf) || currByte == transBuf.put) {
			stopTransmission();
			transmissionComplete = true;
			// TODO: use as sync signal
			GPIOC_BASE->ODR &= ~(1<<8);
			return;
	}
	// Cease transmission if a collision occurs. Prepare to retransmit message
	else if (monitor_getState() == MS_COLLISION) {
		stopTransmission();
		transmissionComplete = false;
		// TODO: use as sync signal
		GPIOC_BASE->ODR &= ~(1<<8);
		return;

	}

	if (currByte == 0 && currBit == 0) {
		// TODO: use as sync signal
		GPIOC_BASE->ODR |= (1<<8);
	}

	// Transmit the one bit by setting its value in the transmission line. bits are sent MSB -> LSB.
	if ((transBuf.buffer[currByte] & (1<<(7-currBit))) != 0)
		select_gpio(TRANSMISSION_GPIO)->ODR |= 1<<TRANSMISSION_PIN;
	else
		select_gpio(TRANSMISSION_GPIO)->ODR &= ~(1<<TRANSMISSION_PIN);

	// increment the bit to transfer for next ISR call
	currBit++;

	// move on to next byte if reached end of byte
	if (currBit == 8) {
		currBit = 0;
		currByte++;
	}

}

/**
 * This converts the byte to manchester encoding, and sets it up for transmission.
*/
static void transmitByte(uint8_t byte) {
	uint16_t manchesterSymbol = encodeManchester(byte);
	// set the uint16_t manchester encoded byte for transmission
	put(&transBuf, manchesterSymbol >> 8);
	put(&transBuf, manchesterSymbol & 0xFF);
}

/**
 * Transmits all of the bytes of the packet header, taking into account
 * that message is not necessarily maximum length
 */
static void transmitMessage(const PacketHeader *pkt) {
	transmitByte(pkt->synch);
	transmitByte(pkt->ver);
	transmitByte(pkt->src);
	transmitByte(pkt->dest);
	transmitByte(pkt->length);
	transmitByte(pkt->crc_flag);
	for (int i = 0; i<pkt->length; i++)
		transmitByte(pkt->msg[i]);
	transmitByte(pkt->crc8_fcs);
}

/**
 * Takes in a byte of data, and converts it to manchester encoding for transmission
 * for each bit:
 * 	0 -> 0b01
 * 	1 -> 0b10
 * This is set so it can be transmitted directly to a pin from bits 0 up to 15,
 * every half-clock period.
 */
static uint16_t encodeManchester(uint8_t data){
	uint16_t output = 0;
	for (int i=0; i<8; i++) {
		int bit = (data & (1<<i)) >> i;
		output |= (0^bit) << (i*2);
		output |= (1^bit) << (i*2+1); // TODO 1 0 -> 0 1
	}
	return output;
}

/**
 * Starts transmission by resetting and enabling the transmission timer's counter.
 * Should only start when in the idle state
 */
static inline void startTransmission() {
	inTransmission = true;
	clear_cnt(TRANSMITTER_TIMER);
	start_counter(TRANSMITTER_TIMER);
}

/**
 * Stop transmission, either due to reaching the end of transmission or due to a collision
 */
static inline void stopTransmission() {
	inTransmission = false;
	select_gpio(TRANSMISSION_GPIO)->ODR |= 1<<TRANSMISSION_PIN;
	stop_counter(TRANSMITTER_TIMER);
}
