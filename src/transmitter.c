#include "transmitter.h"
#include "clock.h"
#include "ringbuffer.h"
#include "tim.h"
#include "gpio.h"
#include "monitor.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>

// buffer of the data sent through UART
static RingBuffer dataBuf = {0, 0};
// transmission buffer to put the manchester encoded bytes in for transmission
static RingBuffer transBuf = {0, 0};
// used by the timer ISR, determines which bit to send at the time of execution
static int currBit = 0;
// TODO: debug for continuously retransmitting message
static int currByte = 0;
static bool inTransmission = false;

// Forward references
static void transmitByte(uint8_t);
static uint16_t encodeManchester(uint8_t);
static void initTransmissionTimer();
static void startTransmission();
static void stopTransmission();

void transmitter_init() {
	init_usart2(19200, F_CPU);
	initTransmissionTimer();

	// Enable transmission pin
	init_GPIO(TRANSMISSION_GPIO);
	enable_output_mode(TRANSMISSION_GPIO, TRANSMISSION_PIN);

	// Debug PC8 - Sync Signal
	init_GPIO(C);
	enable_output_mode(C, 8);
}

void transmitter_mainRoutineUpdate() {
	char c = usart2_getch();
	// only transmit a fully received message from the uart
	static bool gotMessage = false;

	// data to transmit received, transmit it
	if (c == '\r') {
		gotMessage = true;
	}
	// read in data until new line
	else {
		put(&dataBuf, c);
	}

	// start transmission when in TS_IDLE, and there's something to transmit
	if (monitor_IDLE() && !inTransmission && gotMessage) {
		currByte = 0;
		gotMessage = false;
		// transmit all characters
		transBuf.put = transBuf.get = 0;
		while(hasElement(&dataBuf) ){
			c = get(&dataBuf);
			// sets the byte for transmission, or blocks if the transmission buffer is full
			transmitByte(c);
			printf("%c", c);
		}
		// construct the packet buffer
		printf("\r\n");

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
 * This resets itself to transmit as long as the transmission buffer is not empty.
 * It changes the PC9 line every half-clock period in order to match the required bit rate.
 */
void TIM2_IRQHandler(){
	clear_output_cmp_mode_pending_flag(TRANSMITTER_TIMER);

	// Cease transmission if a collision occurs /* TODO or finished transmission*/
	if (monitorState == TS_COLLISION || !hasElement(&transBuf) || currByte == transBuf.put) {
		stopTransmission();
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
