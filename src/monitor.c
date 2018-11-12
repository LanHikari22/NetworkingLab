#include "monitor.h"
#include "delay.h"
#include "gpio.h"
#include "io_definitions.h"
#include <inttypes.h>

// LED Light Status
#define LED_IDLE_PB13 (1<<13)
#define LED_BUSY_PB14 (1<<14)
#define LED_COLLISION_PB15 (1<<15)

// Output mode for LEDs
#define GPIOB_LEDS_OUTPUT_MODE (0b010101 << 26)

// monitor state as observed on monitor line
MONITOR_STATE monitorState;

// either 1 or 0, updated by the pin interrupt
static int lineState;

// This macro lets the reciever handles its own interrupt driven logic with PC9.
// if disabled, no interrupt is hooked on PC9 even though the PC9 line is monitered.
// (instead, the pin interrupt logic monitor_Edge_Intrr is exposed to be used for a pin connected to PC9)
static bool exti9Enable;

static inline void updateMonitorState(MONITOR_STATE newState);

void monitor_start(bool exti9_enable) {
		exti9Enable = exti9_enable;
		// enable GPIOB and set LEDs for updating monitor status
		init_GPIO(B);
		GPIOB_BASE->MODER |= GPIOB_LEDS_OUTPUT_MODE;

		// TODO: debug, sanity check the LEDs are even working
//		*(GPIOB_ODR) |= (0b111<<13);
//		while(1);

		// Enable Clocks for GPIOC
		init_GPIO(C);

		// clear and set PC8 to output for debugging
//		enable_output_mode(C, 8);
//		GPIOC_BASE->ODR &= ~(1<<8);

		updateMonitorState(MS_IDLE);

		setupPinInterrupt();
}

/**
 * returns the monitor state, without exposing the variable outside this module
 */
MONITOR_STATE monitor_getState() {
	return monitorState;
}

void setupPinInterrupt(){
	// Enable Clock to SysCFG
	*(RCC_APB2ENR) |= 1<<14;

	// Enable Clocks for GPIOC
	init_GPIO(C);

	// PC4 is input signal to monitor
	enable_input_mode(C, 4);

	// in case the monitor module handles its own pin interrupt.
	if (exti9Enable) {

		// Connect PC9 to EXTI9
		*(SYSCFG_EXTICR3) &= ~(0b1111<<4);
		*(SYSCFG_EXTICR3) |= (0b0010<<4);

		// Unmask EXTI9 in EXTI
		*(EXTI_IMR) |= 1<<9;

		// Set falling edge
		*(EXTI_FTSR) |= 1<<9;

		// Set to rising edge
		*(EXTI_RTSR) |= 1<<9;

		// Set priority to max TODO: this was wrong, but worked by coincidence (23->24)
		*(NVIC_IPR5) |= 0xF0 << 24;

		// Enable Interrupt in NVIC (Vector table interrupt enable)
		*(NVIC_ISER0) |= 1<<23;
	}
}

// configures the systick timer load value to the specified t_us and resets its val
void configTimer(uint32_t t_us) {
	// set load
	*(STK_LOAD) = 16 * t_us; // 1 us = 16

	// reset val
	*(STK_VAL) = 0;
}

// resets the timer so it time outs after t_us
void resetTimer(uint32_t t_us) {
	// disable timer
	*(STK_CTRL) &= ~(1<<STK_ENABLE_F);

	// configure timer
	// set load
	*(STK_LOAD) = 16 * t_us; // 1 us = 16
	// reset val
	*(STK_VAL) = 0;

	// Turn on counter, and enable interrupt
	*(STK_CTRL) |= (1<<STK_ENABLE_F) | (1<<STK_CLKSOURCE_F) | (1<<STK_TICKINT_F);
}

/**
 * Systick ISR -- used to detect transmission timeout, and sets the state to TS_IDLE or TS_COLLISION
 */
void SysTick_Handler() {
	// disable timer
	*(STK_CTRL) &= ~(1<<STK_ENABLE_F);

	// TODO: DEBUG toggle every timeout period
//	*(GPIOC_ODR) ^= (1<<8);

	// set to TS_IDLE or TS_COLLISION based on line state
	// and update PB13-PB14-PB15
	if(lineState != 0){
		updateMonitorState(MS_IDLE);
	}
	else {
		updateMonitorState(MS_COLLISION);
	}
}

/**
 * EXTI9_5 ISR -- Updates the line state and sets the transmission state to busy
 * also resets the timeout systick timer
 */
void EXTI9_5_IRQHandler() {
	// Verify Interrupt is from EXTI9
	if ((*(EXTI_PR)&(1<<9)) != 0) {
		monitor_Edge_Intrr();
		// Clear Interrupt
		*(EXTI_PR) |= 1<<9;
	}
}

/**
 * triggers on EXTI4. EXTI9 is no longer supported. TODO: remomve that, as this merges with the reciever pin, rather than the transmit pin
 */
void monitor_Edge_Intrr(){
		// reset counter
		resetTimer(TRANSMISSION_TIMEOUT_US);
		// update line state
		lineState = (GPIOC_BASE->IDR &(1<<4))>>4;
		// change state
		updateMonitorState(MS_BUSY);
}

/**
 * if the monitor state is busy, this forces it to collission. This is for testing purposes, mainly.
 */
void monitor_jam(){
	if (monitorState == MS_BUSY) {
		updateMonitorState(MS_COLLISION);
	}
}

/**
 * updates the monitor_state, as well as output signals indicating the state
 */
static inline void updateMonitorState(MONITOR_STATE newState) {
	GPIOB_BASE->ODR &= ~(0b111 << 13);
	switch (newState) {
	case MS_IDLE:
		monitorState = MS_IDLE;
		GPIOB_BASE->ODR |= (LED_IDLE_PB13);
		break;
	case MS_BUSY:
		monitorState = MS_BUSY;
		GPIOB_BASE->ODR |= (LED_BUSY_PB14);
		break;
	case MS_COLLISION:
		monitorState = MS_COLLISION;
		GPIOB_BASE->ODR |= (LED_COLLISION_PB15);
		break;
	}
}
