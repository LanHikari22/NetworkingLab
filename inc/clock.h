/*
 * clock.h
 *
 *  Created on: Sep 18, 2018
 *      Author: liangy
 */

#ifndef CLOCK_H_
#define CLOCK_H_

enum CLK_EN_REGS
{
    AHB1ENR,
    APB1ENR,
    APB2ENR
};

#define F_CPU 16000000UL
#define RCC_AHB1ENR ((volatile uint32_t *) 0x40023830)
#define RCC_APB1ENR ((volatile uint32_t *) 0x40023840)
#define RCC_APB2ENR ((volatile uint32_t *) 0x40023844)

#endif /* CLOCK_H_ */
