/**
 * P01: LED on (bare-metal)
 *
 * Switches on the green LED (LD4) on the STM32F4-Discovery board
 * using direct register access only: no HAL, no CubeMX.
 *
 * Board : STM32F407G-DISC1
 * Chip  : STM32F407VGT6
 * LED   : green LD4 on pin PD12 (UM1472, LEDs section)
 * Clock : 16 MHz internal HSI (default after reset)
 *
 * Reference: RM0090 (STM32F405/407 reference manual)
 */

#include <stdint.h>

/* Base addresses: RM0090, memory map (section 2.3) */
#define RCC_BASE   0x40023800UL     /* RCC: Reset and Clock Control */
#define GPIOD_BASE 0x40020C00UL     /* GPIO port D */

int main(void){
	/* Register pointers: exact address = base + offset */
	volatile uint32_t *const RCC_AHB1ENR = (volatile uint32_t *)(RCC_BASE + 0x30UL);   /* RCC_AHB1ENR: AHB1 peripheral clock enable */
	volatile uint32_t *const GPIOD_MODER = (volatile uint32_t *)(GPIOD_BASE + 0x00UL); /* GPIOx_MODER: pin mode */
	volatile uint32_t *const GPIOD_ODR   = (volatile uint32_t *)(GPIOD_BASE + 0x14UL); /* GPIOx_ODR: output data */

	/* Step 1: switch on port D's clock (RCC_AHB1ENR bit 3, GPIODEN) */
	*RCC_AHB1ENR |= (1UL << 3);
	(void)*RCC_AHB1ENR;             /* read back: short wait so the clock is running (errata ES0182) */

	/* Step 2: make PD12 a general-purpose output (MODER12, bits 25:24 = 01) */
	*GPIOD_MODER &= ~(3UL << 24);   /* clear the field to 00 */
	*GPIOD_MODER |=  (1UL << 24);   /* set it to 01 = output */

	/* Step 3: drive PD12 high (ODR bit 12 = 1): the LED switches on */
	*GPIOD_ODR |= (1UL << 12);

	/* Never return from main: there's no operating system to return to */
	while(1){
	}
}
