/*
 * tim.c
 *
 *  Created on: Sep 23, 2018
 *      Author: liangy
 */

/* All included header files */

#include "tim.h"
#include "clock.h"
#include "isr.h"
#include <inttypes.h>

static void output_cmp_mode(volatile uint32_t *CCMR1);
static void input_capture_mode(volatile uint32_t *CCMR1,
        volatile uint32_t *CCER, enum EDGE_TYPEs edge_type);
static void set_edge_type(volatile uint32_t *CCER, enum EDGE_TYPEs edge_type);

/**
 * Configure the clock for the timer.
 */
void enable_timer_clk(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        *RCC_APB1ENR |= 1 << TIM2EN;
        break;
    case TIM3:
        *RCC_APB1ENR |= 1 << TIM3EN; // enable the timer.
        break;
    case TIM4:
    	*RCC_APB1ENR |= 1 << TIM4EN;
    	break;
    default:
        // Keep empty
        break;
    }
}

/**
 * Sets the value in CCR1.
 * Args:
 * ticks: the number of ticks needs to be set in CCR1.
 */
void set_ccr1(enum TIMs tim, uint32_t ticks)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->CCR1 = ticks;
        break;
    case TIM3:
        TIM3_BASE->CCR1 = ticks;
        break;
    case TIM4:
    	TIM4_BASE->CCR1 = ticks;
    	break;
    default:
        // Keep empty
        break;
    }
}

/**
 * Sets the value in ARR.
 * Args:
 * ticks: the number of ticks needs to be set in ARR.
 */
void set_arr(enum TIMs tim, uint32_t ticks)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->ARR = ticks;
        break;
    case TIM3:
        TIM3_BASE->ARR = ticks;
        break;
    case TIM4:
    	TIM4_BASE->ARR = ticks;
    	break;
    default:
        // Keep empty
        break;
    }
}

void clear_cnt(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->CNT = 0;
        break;
    case TIM3:
        TIM3_BASE->CNT = 0;
        break;
    case TIM4:
    	TIM4_BASE->CNT = 0;
    	break;
    default:
        break;
    }
}

void set_psc(enum TIMs tim, uint32_t ticks)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->PSC = ticks;
        break;
    case TIM3:
        TIM3_BASE->PSC = ticks;
        break;
    case TIM4:
    	TIM4_BASE->PSC = ticks;
    	break;
    default:
        break;
    }
}

/**
 * Logs timer's interrupt into NVIC register.
 */
void log_tim_interrupt(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        *ISER0 |= 1 << TIM2_INTERRUPT_EN;
        break;
    case TIM3:
        *ISER0 |= 1 << TIM3_INTERRUPT_EN;
        break;
    case TIM4:
        *ISER0 |= 1 << TIM4_INTERRUPT_EN;
        break;
    default:
        break;
    }
}

/**
 * Starts counting.
 */
void start_counter(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->CR1 |= 1 << CEN;
        break;
    case TIM3:
        TIM3_BASE->CR1 |= 1 << CEN;
        break;
    case TIM4:
    	TIM4_BASE->CR1 |= 1 << CEN;
    	break;
    default:
        break;
    }
}

/**
 * Stops counting.
 */
void stop_counter(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->CR1 &= ~(1 << CEN);
        break;
    case TIM3:
        TIM3_BASE->CR1 &= ~(1 << CEN);
        break;
    case TIM4:
    	TIM4_BASE->CR1 &= ~(1 << CEN);
    	break;
    default:
        break;
    }
}

/*********This section is for the counter mode************/

void set_to_counter_mode(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->CR1 |= 1 << URS;
        break;
    case TIM3:
        TIM3_BASE->CR1 |= 1 << URS;
        break;
    case TIM4:
        TIM4_BASE->CR1 |= 1 << URS;
        break;
    default:
        break;
    }
}

void enable_counter_mode_interrupt(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->DIER |= 1 << UIE;
        break;
    case TIM3:
        TIM3_BASE->DIER |= 1 << UIE;
        break;
    case TIM4:
        TIM4_BASE->DIER |= 1 << UIE;
        break;
    default:
        break;
    }
}

void disable_counter_mode_interrupt(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->DIER &= ~(1 << UIE);
        break;
    case TIM3:
        TIM3_BASE->DIER &= ~(1 << UIE);
        break;
    case TIM4:
        TIM4_BASE->DIER &= ~(1 << UIE);
        break;
    default:
        break;
    }
}

void clear_counter_mode_pending_flag(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->SR &= ~(1 << UIF);
        break;
    case TIM3:
        TIM3_BASE->SR &= ~(1 << UIF);
        break;
    case TIM4:
        TIM4_BASE->SR &= ~(1 << UIF);
        break;
    default:
        break;
    }
}

/*********************************************************/

/*********This section is for input capture mode**********/

void set_to_input_capture_mode(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        input_capture_mode(&(TIM2_BASE->CCMR1), &(TIM2_BASE->CCER), BOTH);
        break;
    case TIM3:
        input_capture_mode(&(TIM3_BASE->CCMR1), &(TIM3_BASE->CCER), BOTH);
        break;
    case TIM4:
        input_capture_mode(&(TIM4_BASE->CCMR1), &(TIM4_BASE->CCER), BOTH);
        break;
    default:
        break;
    }
}

void enable_input_capture_mode_interrupt(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->DIER |= 1 << CC1IE;
        break;
    case TIM3:
        TIM3_BASE->DIER |= 1 << CC1IE;
        break;
    case TIM4:
        TIM4_BASE->DIER |= 1 << CC1IE;
        break;
    default:
        break;
    }
}

void disable_input_capture_mode_interrupt(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->DIER &= ~(1 << CC1IE);
        break;
    case TIM3:
        TIM3_BASE->DIER &= ~(1 << CC1IE);
        break;
    case TIM4:
        TIM4_BASE->DIER &= ~(1 << CC1IE);
        break;
    default:
        break;
    }
}

void clear_input_capture_mode_pending_flag(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->SR &= ~(1 << CC1IF);
        break;
    case TIM3:
        TIM3_BASE->SR &= ~(1 << CC1IF);
        break;
    case TIM4:
        TIM4_BASE->SR &= ~(1 << CC1IF);
        break;
    default:
        break;
    }
}

/********************************************************/

/*********This section is for output compare mode (Unchecked!!!)**********/

/**
 * Sets the timer into output compare mode.
 */
void set_to_output_cmp_mode(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        output_cmp_mode(&(TIM2_BASE->CCMR1));
        break;
    case TIM3:
        output_cmp_mode(&(TIM3_BASE->CCMR1));
        break;
    case TIM4:
        output_cmp_mode(&(TIM4_BASE->CCMR1));
        break;
    default:
        break;
    }
}

/**
 * Enables the output of the timer.
 */
void enable_output_output_cmp_mode(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->CCER |= 1;
        break;
    case TIM3:
        TIM3_BASE->CCER |= 1;
        break;
    case TIM4:
        TIM4_BASE->CCER |= 1;
        break;
    default:
        break;
    }
}

/**
 * Disable the output of the timer.
 */
void disable_output_output_cmp_mode(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->CCER &= ~1;
        break;
    case TIM3:
        TIM3_BASE->CCER &= ~1;
        break;
    case TIM4:
        TIM4_BASE->CCER &= ~1;
        break;
    default:
        break;
    }
}

/**
 * Clears the pending flag for the interrupt.
 */
void clear_output_cmp_mode_pending_flag(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->SR &= ~(1 << UIF);
        break;
    case TIM3:
        TIM3_BASE->SR &= ~(1 << UIF);
        break;
    case TIM4:
        TIM4_BASE->SR &= ~(1 << UIF);
        break;
    default:
        break;
    }
}

/**
 * Enables the update event interrupt.
 */
void enable_output_cmp_mode_interrupt(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->DIER |= 1;
        break;
    case TIM3:
        TIM3_BASE->DIER |= 1;
        break;
    case TIM4:
        TIM4_BASE->DIER |= 1;
        break;
    default:
        break;
    }
}

/**
 * Disables the update event interrupt.
 */
void disable_output_cmp_mode_interrupt(enum TIMs tim)
{
    switch (tim)
    {
    case TIM2:
        TIM2_BASE->DIER &= ~1;
        break;
    case TIM3:
        TIM3_BASE->DIER &= ~1;
        break;
    case TIM4:
        TIM4_BASE->DIER &= ~1;
        break;
    default:
        break;
    }
}

/********************************************************/

static void output_cmp_mode(volatile uint32_t *CCMR1)
{
	// clear CCMR1.OC1M
    *CCMR1 &= ~(0b111 << 4);
    // 0b011 is Toggle when TIMx_CNT = TIMx_CCR1
    *CCMR1 |= 0b011 << 4;
}

static void input_capture_mode(volatile uint32_t *CCMR1,
        volatile uint32_t *CCER, enum EDGE_TYPEs edge_type)
{
    // Configure the channel to be input
    *CCMR1 &= ~(0b11 << CC1S);
    *CCMR1 |= 0b01 << CC1S;
    // Set the edge type to be both by default
    set_edge_type(CCER, BOTH);
    // Make sure the capture is done each time an edge is detected
    *CCMR1 &= ~(0b11 << IC1PSC);
    // This bit makes the counter value to be in the capture register
    *CCER |= 1 << CC1E;
}

static void set_edge_type(volatile uint32_t *CCER, enum EDGE_TYPEs edge_type)
{
    switch (edge_type)
    {
    case BOTH:
        // Triggers on both edges
        *CCER &= ~(1 << CC1P);
        *CCER |= 1 << CC1P;
        *CCER &= ~(1 << CC1NP);
        *CCER |= 1 << CC1NP;
        break;
    default:
        break;
    }
}
