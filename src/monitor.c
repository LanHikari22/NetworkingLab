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

void setupPinInterrupt();
void SysTick_Handler();
void EXTI9_5_IRQHandler();
void configTimer(uint32_t);
void resetTimer(uint32_t);

TRANSMISSION_STATE monitorState;

// either 1 or 0, updated by the pin interrupt
static int lineState;

void monitor_start() {
		// enable GPIOB and set LEDs for updating monitor status
		init_GPIO(B);
		GPIOB_BASE->MODER |= GPIOB_LEDS_OUTPUT_MODE;

		// TODO: debug, sanity check the LEDs are even working
//		*(GPIOB_ODR) |= (0b111<<13);
//		while(1);

		// Enable Clocks for GPIOC
		init_GPIO(C);

		// clear and set PC8 to output for debugging
		enable_output_mode(C, 8);
		GPIOC_BASE->ODR &= ~(1<<8);

		monitorState = TS_IDLE;
		GPIOB_BASE->ODR &= ~(0b111 << 13);
		GPIOB_BASE->ODR |= (LED_IDLE_PB13);

		setupPinInterrupt();
}

void setupPinInterrupt(){
	// Enable Clock to SysCFG
	*(RCC_APB2ENR) |= 1<<14;

	// Enable Clocks for GPIOC
	init_GPIO(C);

	// TODO: if debugging, set PC9 to input to supply test signals
	enable_input_mode(C, 9);

	// Connect PC9 to EXTI9
	*(SYSCFG_EXTICR3) &= ~(0b1111<<4);
	*(SYSCFG_EXTICR3) |= (0b0010<<4);

	// Unmask EXTI9 in EXTI
	*(EXTI_IMR) |= 1<<9;

	// Set falling edge
	*(EXTI_FTSR) |= 1<<9;

	// Set to rising edge
	*(EXTI_RTSR) |= 1<<9;

	// Set priority to max
	*(NVIC_IPR5) |= 0xFF << 23;

	// Enable Interrupt in NVIC (Vector table interrupt enable)
	*(NVIC_ISER0) |= 1<<23;
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
		monitorState = TS_IDLE;
		GPIOB_BASE->ODR &= ~(0b111 << 13);
		GPIOB_BASE->ODR |= (LED_IDLE_PB13);
	}
	else {
		monitorState = TS_COLLISION;
		GPIOB_BASE->ODR &= ~(0b111 << 13);
		GPIOB_BASE->ODR |= (LED_COLLISION_PB15);
	}
}

/**
 * EXTI9_5 ISR -- Updates the line state and sets the transmission state to busy
 * also resets the timeout systick timer
 */
void EXTI9_5_IRQHandler(){
	// Verify Interrupt is from EXTI9
	if ((*(EXTI_PR)&(1<<9)) != 0) {
		// reset counter
		resetTimer(TRANSMISSION_TIMEOUT_US);
		// Clear Interrupt
		*(EXTI_PR) |= 1<<9;
		// update line state
		lineState = (GPIOC_BASE->IDR &(1<<9))>>9;
		// change state
		monitorState = TS_BUSY;
		GPIOB_BASE->ODR &= ~(0b111 << 13);
		GPIOB_BASE->ODR |= (LED_BUSY_PB14);
	}
}
