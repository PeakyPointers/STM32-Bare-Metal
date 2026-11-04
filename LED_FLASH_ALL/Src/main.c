/**
 * P03a: flash all four LEDs (bare-metal)
 *
 * Switches all four user LEDs on the STM32F4-Discovery on together,
 * then off together, every 500 ms, using direct register access and
 * a busy-wait delay. No HAL, no CubeMX.
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

/* All four LEDs on: set ODR bits 12 to 15 */
void led_set(void){
	for(uint32_t i = 12; i <= 15; i++){
		*GPIOD_ODR |= (1UL << i);
	}
}

/* All four LEDs off: clear ODR bits 12 to 15 */
void led_reset(void){
	for(uint32_t i = 12; i <= 15; i++){
		*GPIOD_ODR &= ~(1UL << i);  /* CHANGED: clear instead of XOR, so the LEDs always end up off */
	}
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

	uint32_t ms = 500;              /* on time and off time */

	/* --- Loop forever --- */
	while(1){
		led_set();                  /* all on */
		delay_ms(ms);
		led_reset();                /* all off */
		delay_ms(ms);
	}
}
