/**
 * P04: button-triggered LED wipe, momentary (bare-metal)
 *
 * While the blue user button (B1) is held, the four user LEDs on the
 * STM32F4-Discovery run a wipe: on one by one, then off one by one,
 * 500 ms per step. When the button is released, all LEDs are switched
 * off. The button is read with GPIOA_IDR; the LEDs are switched with
 * BSRR. Busy-wait delay. No HAL, no CubeMX.
 *
 * Board  : STM32F407G-DISC1
 * Chip   : STM32F407VGT6
 * Button : B1 (USER) on PA0, reads 1 when pressed (UM1472, push-buttons)
 * LEDs   : PD12 green (LD4), PD13 orange (LD3),
 *          PD14 red (LD5), PD15 blue (LD6)   (UM1472, LEDs section)
 * Clock  : 16 MHz internal HSI (default after reset)
 *
 * Reference: RM0090 (STM32F405/407 reference manual)
 */

#include <stdint.h>

/* Base addresses: RM0090, memory map (section 2.3) */
#define RCC_BASE   0x40023800UL     /* RCC: Reset and Clock Control */
#define GPIOD_BASE 0x40020C00UL     /* GPIO port D: LEDs */
#define GPIOA_BASE 0x40020000UL     /* GPIO port A: button */

/* Register pointers: exact address = base + offset */
volatile uint32_t *const RCC_AHB1ENR = (volatile uint32_t *)(RCC_BASE + 0x30UL);   /* RCC_AHB1ENR: AHB1 peripheral clock enable */
volatile uint32_t *const GPIOD_MODER = (volatile uint32_t *)(GPIOD_BASE + 0x00UL); /* GPIOx_MODER: port D pin mode */
volatile uint32_t *const GPIOA_MODER = (volatile uint32_t *)(GPIOA_BASE + 0x00UL); /* GPIOx_MODER: port A pin mode */
volatile uint32_t *const GPIOD_BSRR  = (volatile uint32_t *)(GPIOD_BASE + 0x18UL); /* GPIOx_BSRR: port D bit set/reset (write-only) */
volatile uint32_t *const GPIOA_IDR   = (volatile uint32_t *)(GPIOA_BASE + 0x10UL); /* GPIOx_IDR: port A input data (read-only) */

/* Busy-wait delay of roughly ms milliseconds.
 * About 1000 loop passes per ms at 16 MHz (approximate).
 * 'volatile' stops the compiler from removing the empty loop. */
void delay_ms(uint32_t ms){
	volatile uint32_t i;
	for(i = 0; i <= ms * 1000; i++){
		/* waste time */
	}
}

/* Switch one LED on: write 1 to BSRR set bit j (BSj, bits 0-15), so pin PDj goes high.
 * Writing 0 to the other bits does nothing, so a plain '=' is safe. */
void led_set(uint32_t j){
	*GPIOD_BSRR = (1UL << j);
}

/* Switch one LED off: write 1 to BSRR reset bit k (BRx, bits 16-31).
 * k is the BSRR bit number, not the pin: pin n's reset bit is n + 16,
 * so k = 28 turns off PD12, k = 29 turns off PD13, and so on. */
void led_reset(uint32_t k){
	*GPIOD_BSRR = (1UL << k);
}

int main(void){
	/* --- Setup, done once --- */

	/* Switch on the clocks for port A (bit 0, GPIOAEN) and port D (bit 3, GPIODEN) */
	*RCC_AHB1ENR |= (1UL << 0);
	*RCC_AHB1ENR |= (1UL << 3);
	(void)*RCC_AHB1ENR;             /* read back: short wait so the clocks are running (errata ES0182) */

	/* PA0 (button) as input: MODER0 (bits 1:0) = 00.
	 * Only PA0's bits are touched: PA13/PA14 must stay as the ST-LINK's debug pins. */
	*GPIOA_MODER &= ~(3UL << 00);

	/* PD12 to PD15 (LEDs) as outputs: MODERx fields at bits 24, 26, 28, 30 = 01.
	 * Pin n's 2-bit field starts at bit 2n, so step i by 2. */
	for(uint32_t i = 24; i <= 30; i += 2){
		*GPIOD_MODER &= ~(3UL << i);  /* clear the field to 00 */
		*GPIOD_MODER |=  (1UL << i);  /* set it to 01 = output */
	}

	uint32_t ms = 500;              /* time per step */

	/* --- Loop forever --- */
	while(1){
		/* Momentary: act only while the button is held (IDR bit 0 = 1) */
		if(*GPIOA_IDR & (1UL << 0)){
			/* Fill: switch LEDs on one by one, PD12 → PD15 (set bits 12-15) */
			for(uint32_t j = 12; j <= 15; j++){
				led_set(j);
				delay_ms(ms);
			}
			/* Empty: switch LEDs off one by one, PD12 → PD15 (reset bits 28-31) */
			for(uint32_t k = 28; k <= 31; k++){
				led_reset(k);
				delay_ms(ms);
			}
		}
		else{
			/* Button released: make sure all four LEDs are off */
			for(uint32_t l = 28; l <= 31; l++){
				led_reset(l);
			}
		}
	}
}
