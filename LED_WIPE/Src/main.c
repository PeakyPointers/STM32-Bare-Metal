/**
 * P03c: LED wipe (bare-metal)
 *
 * Switches the four user LEDs on the STM32F4-Discovery on one by one
 * until all are lit, then off one by one in the same order, 500 ms per
 * step, using direct register access and a busy-wait delay.
 * No HAL, no CubeMX.
 *
 * Board : STM32F407G-DISC1
 * Chip  : STM32F407VGT6
 * LEDs  : PD12 green (LD4), PD13 orange (LD3),
 *         PD14 red (LD5), PD15 blue (LD6)   (UM1472, LEDs section)
 * Clock : 16 MHz internal HSI (default after reset)
 *
 * Reference: RM0090 (STM32F405/407 reference manual)
 */

#include <stdint.h>

/* Base addresses: RM0090, memory map (section 2.3) */
#define RCC_BASE   0x40023800UL     /* RCC: Reset and Clock Control */
#define GPIOD_BASE 0x40020C00UL     /* GPIO port D */

/* Register pointers: exact address = base + offset */
volatile uint32_t *const RCC_AHB1ENR = (volatile uint32_t *)(RCC_BASE + 0x30UL);   /* RCC_AHB1ENR: AHB1 peripheral clock enable */
volatile uint32_t *const GPIOD_MODER = (volatile uint32_t *)(GPIOD_BASE + 0x00UL); /* GPIOx_MODER: pin mode */
volatile uint32_t *const GPIOD_ODR   = (volatile uint32_t *)(GPIOD_BASE + 0x14UL); /* GPIOx_ODR: output data */

/* Busy-wait delay of roughly ms milliseconds.
 * About 1000 loop passes per ms at 16 MHz (approximate).
 * 'volatile' stops the compiler from removing the empty loop. */
void delay_ms(uint32_t ms){
	volatile uint32_t i;
	for(i = 0; i <= ms * 1000; i++){
		/* waste time */
	}
}

/* Switch one LED on: set ODR bit j (pin PDj high) */
void led_set(uint32_t j){
	*GPIOD_ODR |= (1UL << j);
}

/* Switch one LED off: clear ODR bit k (pin PDk low) */
void led_reset(uint32_t k){
	*GPIOD_ODR &= ~(1UL << k);
}

int main(void){
	/* --- Setup, done once --- */

	/* Switch on port D's clock (RCC_AHB1ENR bit 3, GPIODEN) */
	*RCC_AHB1ENR |= (1UL << 3);
	(void)*RCC_AHB1ENR;             /* read back: short wait so the clock is running (errata ES0182) */

	/* Make PD12 to PD15 outputs: MODERx fields at bits 24, 26, 28, 30 = 01.
	 * Pin n's 2-bit field starts at bit 2n, so step i by 2. */
	for(uint32_t i = 24; i <= 30; i += 2){
		*GPIOD_MODER &= ~(3UL << i);  /* clear the field to 00 */
		*GPIOD_MODER |=  (1UL << i);  /* set it to 01 = output */
	}

	uint32_t ms = 500;              /* time per step */

	/* --- Loop forever --- */
	while(1){
		/* Fill: switch LEDs on one by one, PD12 → PD15 */
		for(uint32_t j = 12; j <= 15; j++){
			led_set(j);
			delay_ms(ms);
		}
		/* Empty: switch LEDs off one by one, PD12 → PD15 */
		for(uint32_t k = 12; k <= 15; k++){
			led_reset(k);
			delay_ms(ms);
		}
	}
}
