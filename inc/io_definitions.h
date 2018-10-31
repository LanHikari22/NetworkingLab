/**
 * @author Mohammed Alzakariya
 * This header file is to group common I/O registers used but not yet put into their own dedicated modules for handling
 */
#ifndef IO_DEFINITIONS
#define IO_DEFINITIONS

// **SYSCFG**
#define SYSCFG_EXTICR2	(volatile uint32_t*)0x4001380C
#define SYSCFG_EXTICR3	(volatile uint32_t *)0x40013810

// ***EXTI registers*** (section 10.3 of the STM32F466 reference manual)
#define EXTI_IMR		(volatile uint32_t *)0x40013c00 // Interrupt mask register
#define EXTI_FTSR       (volatile uint32_t *)0x40013c0c // Falling trigger select register
#define EXTI_RTSR		(volatile uint32_t *)0x40013c08
#define EXTI_PR         (volatile uint32_t *)0x40013c14 // Interrupt pending register

// ***RCC registers***
#define RCC_APB2ENR	    (volatile uint32_t *)0x40023844

// ***SysTick***
#define F_CPU 16000000UL
#define STK_CTRL		(volatile uint32_t*) 0xE000E010
#define STK_LOAD 		(volatile uint32_t*) 0xE000E014
#define STK_VAL 		(volatile uint32_t*) 0xE000E018
#define STK_ENABLE_F 0
#define STK_TICKINT_F 1
#define STK_CLKSOURCE_F 2
#define STK_CNTFLAG_F 16

// **NVIC**
#define NVIC_ISER0 		((volatile uint32_t*)0xE000E100)
#define NVIC_IPR0 		((volatile uint32_t*)0xE000E400)
#define NVIC_IPR1 		((volatile uint32_t*)0xE000E404)
#define NVIC_IPR2 		((volatile uint32_t*)0xE000E408)
#define NVIC_IPR3 		((volatile uint32_t*)0xE000E40C)
#define NVIC_IPR4 		((volatile uint32_t*)0xE000E410)
#define NVIC_IPR5 		((volatile uint32_t*)0xE000E414)
#define NVIC_ISER1 		((volatile uint32_t*)0xE000E11C)


#endif // IO_DEFINITIONS
