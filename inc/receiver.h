#ifndef RECEIVER_H
#define RECEIVER_H

#include "tim.h"
#include <stdbool.h>

// The transmission bit rate dictates the ticks used for the timers
// The maximum period allowed, is at the lowest frequency allowed, which is bps - 1.3%bps = 986.8bps
// ticks = 16E6 / (f_hz - 1.3%f_hz) / 2
#define HALFBIT_TIMEOUT_TICKS	8107 // 986.8 bps, this amount of tricks correspond approximately to 507us.

// Input pin used for receive
#define RECEIVE_GPIO	C
#define RECEIVE_PIN		9

// initiates the receiver module
void receiver_init(bool packet_mode);

// Main routine update, this should execute inside a while(1); by what uses this module.
void receiver_mainRoutineUpdate();

// Input Capture Timer ISR used for receiving. Update if using a different timer
void TIM3_IRQHandler();

// Counter Timer for Half bit timeout. Indicates whether to sample on the next half clock period or not
void TIM4_IRQHandler();

#endif // RECEIVER_H
