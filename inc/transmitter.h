#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include "tim.h"

// Timer dedicated for the transmission
// Update the corresponding ISR handler as well
#define TRANSMITTER_TIMER TIM2

// The transmission bit rate dictates the ticks used for the timers
// ticks = 1E6/f_hz / 0.0625 / 2
#define TRANSMISSION_TICKS	8000 // 1000 bps

// Output pin used for transmission
#define TRANSMISSION_GPIO	C
#define TRANSMISSION_PIN	9

// initiates the transmitter module
void transmitter_init();

// Main routine update, this should execute inside a while(1); by what uses this module.
void transmitter_mainRoutineUpdate();

// Timer ISR used for transmission. Update if using a different timer
void TIM2_IRQHandler();

#endif // TRANSMITTER_H
