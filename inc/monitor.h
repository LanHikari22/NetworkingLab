/**
 * @file monitor.h
 * This is the header file for the monitor module which exposes its API.
 * The monitor uses:
 * - the systick
 * - an external interrupt on EXTI9_5 (PC9) in order to monitor transmission
 * - PB13-PB14-PB15 to real-time output the monitor state, this maps to TS_IDLE, TS_BUSY, TS_COLLISION
 */

// this configures the monitor so that it can run

#ifndef MONITOR_H
#define MONITOR_H

#include <stdbool.h>

typedef enum {
	TS_IDLE,
	TS_BUSY,
	TS_COLLISION
} TRANSMISSION_STATE;

// The current state of the monitor as it monitors PC9
extern TRANSMISSION_STATE monitorState;

// The period of time until a data transmission timeout occurs
// The monitor enters the TS_IDLE or TS_COLLISION states when that happens
#define TRANSMISSION_TIMEOUT_US 1110

void monitor_start();

// returns true if in IDLE state by checking the idle signal
bool monitor_IDLE();

// returns true if in BUSY state by checking the busy signal
bool monitor_BUSY();

// returns true if in COLLISION state by checking collision signal
bool monitor_COLLISION();

#endif // MONITOR_H
