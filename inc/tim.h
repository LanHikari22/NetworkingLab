/*
 * tim.h
 *
 *  Created on: Sep 23, 2018
 *      Author: liangy
 */

#ifndef TIM_H_
#define TIM_H_

#include "clock.h"
#include <inttypes.h>

// Enumerators for indicating timers
enum TIMs
{
    TIM1,
    TIM2,
    TIM3,
    TIM4,
    TIM5,
    TIM6,
    TIM7,
    TIM8,
    TIM9,
    TIM10,
    TIM11,
    TIM12,
    TIM13,
    TIM14
};

// Edge types
enum EDGE_TYPEs
{
    RISING, FALLING, BOTH
};

/* The structure for TIMER2 to 5 */
typedef struct
{
    uint32_t CR1;
    uint32_t CR2;
    uint32_t SMCR;
    uint32_t DIER;
    uint32_t SR;
    uint32_t EGR;
    uint32_t CCMR1;
    uint32_t CCMR2;
    uint32_t CCER;
    uint32_t CNT;
    uint32_t PSC;
    uint32_t ARR;
    uint32_t reserved1;
    uint32_t CCR1;
    uint32_t CCR2;
    uint32_t CCR3;
    uint32_t CCR4;
    uint32_t reserved2;
    uint32_t DCR;
    uint32_t DMAR;
} TIMER2to5;

/* The structure for TIMER9 to 14 */
typedef struct
{
    uint32_t CR1;
    uint32_t reserved1;
    uint32_t SMCR;
    uint32_t DIER;
    uint32_t SR;
    uint32_t EGR;
    uint32_t CCMR1;
    uint32_t reserved2;
    uint32_t CCER;
    uint32_t CNT;
    uint32_t PSC;
    uint32_t ARR;
    uint32_t reserved3;
    uint32_t CCR1;
    uint32_t CCR2;
} TIMER9to14;

// Clock enable bits
#define TIM1EN  0
#define TIM2EN  0
#define TIM3EN  1
#define TIM4EN  2
#define TIM5EN  3
#define TIM6EN  4
#define TIM7EN  5
#define TIM8EN  1
#define TIM9EN  16
#define TIM10EN 17
#define TIM11EN 18
#define TIM12EN 6
#define TIM13EN 7
#define TIM14EN 8

// Timer based addresses
#define TIM2_BASE ((volatile TIMER2to5 *) 0x40000000)
#define TIM3_BASE ((volatile TIMER2to5 *) 0x40000400)
#define TIM9_BASE ((volatile TIMER2to5 *) 0x40014000)

// The interrupt enable bits
#define TIM2_INTERRUPT_EN 28
#define TIM3_INTERRUPT_EN 29
#define TIM4_INTERRUPT_EN 30
#define TIM5_INTERRUPT_EN 18
#define TIM9_INTERRUPT_EN 24

// CR1 bits
#define CEN     0
#define URS     2
// CCMR1 bits
#define IC1PSC  2
#define CC1S    0
// CCER bits
#define CC1P    1
#define CC1NP   3
#define CC1E    0
// DIER bits
#define UIE     0
#define CC1IE   1
// SR bits
#define UIF     0
#define CC1IF   1

/**
 * Enable the system clock for one timer
 */
void enable_timer_clk(enum TIMs tim);

/**
 * Sets the value for CCR1 register
 *
 */
void set_ccr1(enum TIMs tim, uint32_t ticks);

/**
 * Sets the value in ARR.
 * Args:
 * ticks: the number of ticks needs to be set in ARR.
 */
void set_arr(enum TIMs tim, uint32_t ticks);

/**
 * sets CNT to 0, can be useful for one-pulse mode?
 */
void clear_cnt(enum TIMs tim);


/**
 * Logs timer's interrupt into NVIC register.
 */
void log_tim_interrupt(enum TIMs tim);

/**
 * Starts counting.
 */
void start_counter(enum TIMs tim);

/**
 * Stops counting.
 */
void stop_counter(enum TIMs tim);

void set_psc(enum TIMs tim, uint32_t ticks);

/*********This section is for the counter mode************/

void set_to_counter_mode(enum TIMs tim);

void enable_counter_mode_interrupt(enum TIMs tim);

void disable_counter_mode_interrupt(enum TIMs tim);

void clear_counter_mode_pending_flag(enum TIMs tim);

/*********************************************************/

/*********This section is for input capture mode**********/

void set_to_input_capture_mode(enum TIMs tim);

void enable_input_capture_mode_interrupt(enum TIMs tim);

void disable_input_capture_mode_interrupt(enum TIMs tim);

void clear_input_capture_mode_pending_flag(enum TIMs tim);

/********************************************************/

/*********This section is for output compare mode (Unchecked!!!)**********/

/**
 * Sets the timer into output compare mode.
 */
void set_to_output_cmp_mode(enum TIMs tim);

/**
 * Enables the output of the timer.
 */
void enable_output_output_cmp_mode(enum TIMs tim);

/**
 * Disable the output of the timer.
 */
void disable_output_output_cmp_mode(enum TIMs tim);

/**
 * Enables the update event interrupt.
 */
void enable_output_cmp_mode_interrupt(enum TIMs tim);

/**
 * Disables the update event interrupt.
 */
void disable_output_cmp_mode_interrupt(enum TIMs tim);

/**
 * Clears the pending flag for the interrupt.
 */
void clear_output_cmp_mode_pending_flag(enum TIMs tim);

/********************************************************/

#endif /* TIM_H_ */
