/**
 * P02: blink an LED (bare-metal)
 *
 * Blinks the green LED (LD4) on the STM32F4-Discovery board:
 * 500 ms on, 500 ms off, using direct register access and a
 * busy-wait delay. No HAL, no CubeMX.
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

/* Register pointers: exact address = base + offset, visible to all functions */
volatile uint32_t *const RCC_AHB1ENR = (volatile uint32_t *)(RCC_BASE + 0x30UL);   /* RCC_AHB1ENR: AHB1 peripheral clock enable */
volatile uint32_t *const GPIOD_MODER = (volatile uint32_t *)(GPIOD_BASE + 0x00UL); /* GPIOx_MODER: pin mode */
volatile uint32_t *const GPIOD_ODR   = (volatile uint32_t *)(GPIOD_BASE + 0x14UL); /* GPIOx_ODR: output data */

/* LED on: set ODR bit 12, so PD12 goes high */
void led_set(void){
	*GPIOD_ODR |= (1UL << 12);
}

/* LED off: clear ODR bit 12, so PD12 goes low */
void led_reset(void){
	*GPIOD_ODR &= ~(1UL << 12);
}

/* Busy-wait delay of roughly ms milliseconds.
 * 2000 loop passes is about 1 ms at 16 MHz (calibrated by stopwatch).
 * 'volatile' stops the compiler from removing the empty loop. */
void delay_ms(uint32_t ms){
	volatile uint32_t i;
	for(i = 0; i <= ms * 2000; i++){
		/* waste time */
	}
}

int main(void){
	/* --- Setup, done once --- */

	/* Switch on port D's clock (RCC_AHB1ENR bit 3, GPIODEN) */
	*RCC_AHB1ENR |= (1UL << 3);
	(void)*RCC_AHB1ENR;             /* read back: short wait so the clock is running (errata ES0182) */

	/* Make PD12 a general-purpose output (MODER12, bits 25:24 = 01) */
	*GPIOD_MODER &= ~(3UL << 24);   /* clear the field to 00 */
	*GPIOD_MODER |=  (1UL << 24);   /* set it to 01 = output */

	uint32_t ms = 500;              /* half of the blink period */

	/* --- Loop forever --- */
	while(1){
		led_set();                  /* on */
		delay_ms(ms);
		led_reset();                /* off */
		delay_ms(ms);
	}
}
