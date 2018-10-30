/*
 * led.c
 *
 *  Created on: Sep 18, 2018
 *      Author: liangy
 */

#include "clock.h"
#include "gpio.h"
#include <inttypes.h>

static void init_PA7_to_PA11();
static void init_PB8_to_PB10_and_PB12_to_PB13();

/**
 * Initializes all LEDs used in this program.
 */
void light_LED_init()
{
    init_PA7_to_PA11();
    init_PB8_to_PB10_and_PB12_to_PB13();
}

/**
 * Lights up the corresponding LED based on the number passed in.
 */
void light_LED(uint16_t number)
{
    uint16_t least_5_bits = (number & ~0xFFE0) << 7;
    uint16_t middle_3_bits = (number & ~0xFF1F) << 3;
    uint16_t last_2_bits = (number & ~0xFCFF) << 4;

    GPIOA_BASE->ODR &= 0; // clear all bits to 0.
    GPIOB_BASE->ODR &= 0;
    GPIOA_BASE->ODR |= least_5_bits;
    GPIOB_BASE->ODR |= middle_3_bits;
    GPIOB_BASE->ODR |= last_2_bits;
}

/*
 * Turn on LED using index
 */
void led_on(uint32_t led_num)
{
    // Switch statement for turning on which LED
    /*
     * GPIO A LED PINS : 7 8 9 10 11
     * GPIO B LED PINS : 8 9 10 12 13
     */
    switch (led_num)
    {
    case 0:
        GPIOA_BASE->ODR |= 0x0080;
        break;
    case 1:
        GPIOA_BASE->ODR |= 0x0100;
        break;
    case 2:
        GPIOA_BASE->ODR |= 0x0200;
        break;
    case 3:
        GPIOA_BASE->ODR |= 0x0400;
        break;
    case 4:
        GPIOA_BASE->ODR |= 0x0800;
        break;
    case 5:
        GPIOB_BASE->ODR |= 0x0100;
        break;
    case 6:
        GPIOB_BASE->ODR |= 0x0200;
        break;
    case 7:
        GPIOB_BASE->ODR |= 0x0400;
        break;
    case 8:
        GPIOB_BASE->ODR |= 0x1000;
        break;
    case 9:
        GPIOB_BASE->ODR |= 0x2000;
        break;
    default:
        break;
    }
}

/*
 * Turn off LED using index
 */
void led_off(uint32_t led_num)
{
    // Switch statement for turning off which LED
    /*
     * GPIO A LED PINS : 7 8 9 10 11
     * GPIO B LED PINS : 8 9 10 12 13


     */
    switch (led_num)
    {
    case 0:
        GPIOA_BASE->ODR &= ~0x0080;
        break;
    case 1:
        GPIOA_BASE->ODR &= ~0x0100;
        break;
    case 2:
        GPIOA_BASE->ODR &= ~0x0200;
        break;
    case 3:
        GPIOA_BASE->ODR &= ~0x0400;
        break;
    case 4:
        GPIOA_BASE->ODR &= ~0x0800;
        break;
    case 5:
        GPIOB_BASE->ODR &= ~0x0100;
        break;
    case 6:
        GPIOB_BASE->ODR &= ~0x0200;
        break;
    case 7:
        GPIOB_BASE->ODR &= ~0x0400;
        break;
    case 8:
        GPIOB_BASE->ODR &= ~0x1000;
        break;
    case 9:
        GPIOB_BASE->ODR &= ~0x2000;
        break;
    default:
        break;
    }
}

/**
 * Initializes the first five LEDs.
 */
static void init_PA7_to_PA11()
{
    /* enable the clock for GPIOA */
    *RCC_AHB1ENR |= 1;
    /* set mode */
    GPIOA_BASE->MODER &= (~0b1111111111 << 14); // make sure bits are set to 0.
    GPIOA_BASE->MODER |= (0b0101010101 << 14);
}

/**
 * Initializes the last five LEDs.
 */
static void init_PB8_to_PB10_and_PB12_to_PB13()
{
    /* enable the clock for GPIOB */
    *RCC_AHB1ENR |= (1 << 1);
    /* set mode */
    GPIOB_BASE->MODER &= (~0b111111 << 16);
    GPIOB_BASE->MODER &= (~0b1111 << 24);
    GPIOB_BASE->MODER |= (0b010101 << 16);
    GPIOB_BASE->MODER |= (0b0101 << 24);
}

