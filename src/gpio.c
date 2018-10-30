/*
 * gpio.c
 *
 *  Created on: Sep 25, 2018
 *      Author: liangy
 */

#include "gpio.h"
#include "clock.h"

void init_GPIO(enum GPIOs gpio)
{
    switch (gpio)
    {
    case A:
        *RCC_AHB1ENR |= 1 << GPIOA_EN;
        break;
    case B:
        *RCC_AHB1ENR |= 1 << GPIOB_EN;
        break;
    case C:
        *RCC_AHB1ENR |= 1 << GPIOC_EN;
        break;
    case D:
        *RCC_AHB1ENR |= 1 << GPIOD_EN;
        break;
    default:
        break;
    }
}

void enable_input_mode(enum GPIOs gpio, int pin)
{
	volatile GPIO *gpio_ptr = select_gpio(gpio);

    gpio_ptr->MODER &= ~(0b11 << 2*pin);
}


void enable_output_mode(enum GPIOs gpio, int pin)
{
	volatile GPIO *gpio_ptr = select_gpio(gpio);

    gpio_ptr->MODER &= ~(0b11 << 2*pin);
    gpio_ptr->MODER |= 1 << 2*pin;

}

void enable_af_mode(enum GPIOs gpio, int pin, int af_num)
{
	volatile GPIO *gpio_ptr = select_gpio(gpio);
    //Setup GPIOA PIN 5 to Alternate Mode
    gpio_ptr->MODER &= 0xFFFFFBFF;
    gpio_ptr->MODER |= 0x00000800;
    //Setup PIN 5 to AF1
    if (pin <= 7)
    {
        gpio_ptr->AFRL &= ~(0xF << (4 * pin));
        gpio_ptr->AFRL |= (af_num << (4 * pin));
    }
    else if (pin > 7 && pin <= 15)
    {
        gpio_ptr->AFRH &= ~(0xF << (4 * (pin - 8)));
        gpio_ptr->AFRH |= (af_num << (4 * (pin - 8)));
    }
}

inline volatile GPIO* select_gpio(enum GPIOs gpio) {
    volatile GPIO *gpio_ptr = 0;
    switch (gpio)
    {
    case A:
        gpio_ptr = GPIOA_BASE;
        break;
    case B:
        gpio_ptr = GPIOB_BASE;
        break;
    case C:
        gpio_ptr = GPIOC_BASE;
        break;
    case D:
    	gpio_ptr = GPIOD_BASE;
    	break;
    default:
        break;
    }
    return gpio_ptr;
}
