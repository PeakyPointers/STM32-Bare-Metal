/**
 * P05: latched button toggle with simple debounce (bare-metal)
 *
 * Each press of the blue user button (B1) toggles the green LED (LD4):
 * press once and it turns on and stays on, press again and it turns off.
 * Presses are detected on the rising edge (released → pressed), and a
 * 50 ms wait after each "pressed" reading filters out contact bounce.
 * Button read with GPIOA_IDR; LED switched with BSRR. No HAL, no CubeMX.
 *
 * Board  : STM32F407G-DISC1
 * Chip   : STM32F407VGT6
 * Button : B1 (USER) on PA0, reads 1 when pressed (UM1472, push-buttons)
 * LED    : PD12 green (LD4)   (UM1472, LEDs section)
 * Clock  : 16 MHz internal HSI (default after reset)
 *
 * Reference: RM0090 (STM32F405/407 reference manual)
 */
#include <stdint.h>
#include <stdbool.h>

/* Base addresses: RM0090, memory map (section 2.3) */
#define RCC_BASE   0x40023800UL     /* RCC: Reset and Clock Control */
#define GPIOD_BASE 0x40020C00UL     /* GPIO port D: LED */
#define GPIOA_BASE 0x40020000UL     /* GPIO port A: button */

/* Register pointers: exact address = base + offset */
volatile uint32_t *const RCC_AHB1ENR = (volatile uint32_t *)(RCC_BASE + 0x30UL);   /* RCC_AHB1ENR: AHB1 peripheral clock enable */
volatile uint32_t *const GPIOD_MODER = (volatile uint32_t *)(GPIOD_BASE + 0x00UL); /* GPIOx_MODER: port D pin mode */
volatile uint32_t *const GPIOA_MODER = (volatile uint32_t *)(GPIOA_BASE + 0x00UL); /* GPIOx_MODER: port A pin mode */
volatile uint32_t *const GPIOD_BSRR  = (volatile uint32_t *)(GPIOD_BASE + 0x18UL); /* GPIOx_BSRR: port D bit set/reset (write-only) */
volatile uint32_t *const GPIOA_IDR   = (volatile uint32_t *)(GPIOA_BASE + 0x10UL); /* GPIOx_IDR: port A input data (read-only) */

/* Busy-wait delay of roughly ms milliseconds, then return true.
 * About 1000 loop passes per ms at 16 MHz (approximate).
 * 'volatile' stops the compiler from removing the empty loop.
 * Returning true lets it be used inside an if-condition. */
bool delay_ms(uint32_t ms){
	volatile uint32_t i;
	for(i = 0; i <= ms * 1000; i++){
		/* waste time */
	}
	return 1;
}

/* Switch one LED on: write 1 to BSRR set bit j (BSj, bits 0-15), so pin PDj goes high.
 * Writing 0 to the other bits does nothing, so a plain '=' is safe. */
void led_set(uint32_t j){
	*GPIOD_BSRR = (1UL << j);
}

/* Switch one LED off: write 1 to BSRR reset bit k (BRx, bits 16-31).
 * k is the BSRR bit number, not the pin: pin n's reset bit is n + 16,
 * so k = 28 turns off PD12. */
void led_reset(uint32_t k){
	*GPIOD_BSRR = (1UL << k);
}

int main(void){
	/* --- Setup, done once --- */
	uint32_t ms = 50;               /* debounce wait */
	uint32_t i = 12, j = 28;        /* PD12: BSRR set bit 12, reset bit 28 */

	/* Switch on the clocks for port A (bit 0, GPIOAEN) and port D (bit 3, GPIODEN) */
	*RCC_AHB1ENR |= (1UL << 0);
	*RCC_AHB1ENR |= (1UL << 3);
	(void)*RCC_AHB1ENR;             /* read back: short wait so the clocks are running (errata ES0182) */

	/* PA0 (button) as input: MODER0 (bits 1:0) = 00.
	 * Only PA0's bits are touched: PA13/PA14 must stay as the ST-LINK's debug pins. */
	*GPIOA_MODER &= ~(3UL << 00);

	/* PD12 (LED) as output: MODER12 (bits 25:24) = 01 */
	*GPIOD_MODER &= ~(3UL << 24);  /* clear the field to 00 */
	*GPIOD_MODER |=  (1UL << 24);  /* set it to 01 = output */

	bool last_button_state = false;     /* button reading on the previous pass */
	bool latest_button_state = false;   /* button reading on this pass */
	bool button_state_latched = false;  /* LED state: toggled on each press */

	/* --- Loop forever --- */
	while(1){
		/* Read the button. When it reads pressed (IDR bit 0 = 1), the '&&' also
		 * runs delay_ms(50) before accepting it, which waits out contact bounce. */
		if((*GPIOA_IDR & (1UL << 0)) && delay_ms(ms)){
			latest_button_state = true;
		}
		else{
			latest_button_state = false;
		}

		/* Rising edge: pressed now, released on the previous pass → toggle once */
		if(latest_button_state && !last_button_state){
			button_state_latched = !button_state_latched;
		}

		/* Drive the LED from the latched state */
		if(button_state_latched){
			led_set(i);             /* PD12 on */
		}
		else{
			led_reset(j);           /* PD12 off */
		}

		/* This pass's reading becomes the previous one for the next pass */
		last_button_state = latest_button_state;
	}
}
