/**
 * P06: SysTick millisecond delay (bare-metal)
 *
 * Replaces the calibrated busy-wait loop with an accurate delay based on the
 * Cortex-M4 SysTick timer: SysTick wraps every 1 ms, and timer_delay(ms)
 * waits for ms wraps. Demonstrated with the latched blink mode from P05:
 * each press of the blue user button (B1) starts or stops all four user LEDs
 * blinking together (500 ms on, 500 ms off). No HAL, no CubeMX.
 *
 * Board  : STM32F407G-DISC1
 * Chip   : STM32F407VGT6
 * Button : B1 (USER) on PA0, reads 1 when pressed (UM1472, push-buttons)
 * LEDs   : PD12 green (LD4), PD13 orange (LD3),
 *          PD14 red (LD5), PD15 blue (LD6)   (UM1472, LEDs section)
 * Clock  : 16 MHz internal HSI (default after reset)
 *
 * Reference: RM0090 (STM32F405/407 reference manual),
 *            PM0214 (STM32 Cortex-M4 programming manual, SysTick)
 */
#include <stdint.h>
#include <stdbool.h>

/* Base addresses: RM0090 memory map (section 2.3); SysTick: PM0214 */
#define RCC_BASE   0x40023800UL     /* RCC: Reset and Clock Control */
#define GPIOD_BASE 0x40020C00UL     /* GPIO port D: LEDs */
#define GPIOA_BASE 0x40020000UL     /* GPIO port A: button */
#define STK_BASE   0xE000E010UL     /* SysTick timer (Cortex-M4 core) */

/* Register pointers: exact address = base + offset */
volatile uint32_t *const RCC_AHB1ENR = (volatile uint32_t *)(RCC_BASE + 0x30UL);   /* RCC_AHB1ENR: AHB1 peripheral clock enable */
volatile uint32_t *const GPIOD_MODER = (volatile uint32_t *)(GPIOD_BASE + 0x00UL); /* GPIOx_MODER: port D pin mode */
volatile uint32_t *const GPIOA_MODER = (volatile uint32_t *)(GPIOA_BASE + 0x00UL); /* GPIOx_MODER: port A pin mode */
volatile uint32_t *const GPIOD_BSRR  = (volatile uint32_t *)(GPIOD_BASE + 0x18UL); /* GPIOx_BSRR: port D bit set/reset (write-only) */
volatile uint32_t *const GPIOA_IDR   = (volatile uint32_t *)(GPIOA_BASE + 0x10UL); /* GPIOx_IDR: port A input data (read-only) */
volatile uint32_t *const STK_CTRL    = (volatile uint32_t *)(STK_BASE + 0x00UL);   /* STK_CTRL: SysTick control and status */
volatile uint32_t *const STK_LOAD    = (volatile uint32_t *)(STK_BASE + 0x04UL);   /* STK_LOAD: SysTick reload value */
volatile uint32_t *const STK_VAL     = (volatile uint32_t *)(STK_BASE + 0x08UL);   /* STK_VAL: SysTick current value */

/* Configure SysTick to wrap every 1 ms at 16 MHz:
 * - LOAD = 16,000 - 1 = 15,999 (0x3E7F): one wrap every 16,000 clock ticks
 * - VAL  = 0: clear the current count (and COUNTFLAG) so the first wrap is a full 1 ms
 * - CTRL = 0x05: bit 2 CLKSOURCE = processor clock, bit 0 ENABLE = on,
 *   bit 1 TICKINT = 0 (no interrupt; the flag is polled instead) */
void systick_init(){
	*STK_LOAD = 0x3E7FUL;
	*STK_VAL = 0x00UL;
	*STK_CTRL = 0x05UL;
}

/* Wait for one SysTick wrap (1 ms): poll COUNTFLAG (STK_CTRL bit 16) until it reads 1.
 * Reading STK_CTRL clears COUNTFLAG, ready for the next millisecond. */
bool timer_1ms(){
	while(!(*STK_CTRL & (1UL << 16))){
		/* wait */
	}
	return 1;
}

/* Blocking delay of ms milliseconds: wait for ms SysTick wraps, then return true.
 * Returning true lets it be used inside an if-condition. */
bool timer_delay(uint32_t ms){
	for(uint32_t i = 0; i < ms; i++){
		timer_1ms();
	}
	return 1;
}

/* All four LEDs on: write 1 to BSRR set bits 12-15 (BS12 to BS15), PD12 to PD15 go high.
 * Writing 0 to the other bits does nothing, so a plain '=' is safe. */
void led_set(){
	for(uint32_t j = 12; j <= 15; j++){
		*GPIOD_BSRR = (1UL << j);
	}
}

/* All four LEDs off: write 1 to BSRR reset bits 28-31 (BR12 to BR15).
 * Pin n's reset bit is n + 16, so bits 28-31 turn off PD12 to PD15. */
void led_reset(){
	for(uint32_t k = 28; k <= 31; k++){
		*GPIOD_BSRR = (1UL << k);
	}
}

int main(void){
	/* --- Setup, done once --- */

	/* SysTick: 1 ms timebase (part of the Cortex-M4 core, no RCC enable needed) */
	systick_init();

	/* Switch on the clocks for port A (bit 0, GPIOAEN) and port D (bit 3, GPIODEN) */
	*RCC_AHB1ENR |= (1UL << 0);
	*RCC_AHB1ENR |= (1UL << 3);
	(void)*RCC_AHB1ENR;             /* read back: short wait so the clocks are running (errata ES0182) */

	/* PA0 (button) as input: MODER0 (bits 1:0) = 00.
	 * Only PA0's bits are touched: PA13/PA14 must stay as the ST-LINK's debug pins. */
	*GPIOA_MODER &= ~(3UL << 00);

	bool last_button_state = false;     /* button reading on the previous pass */
	bool latest_button_state = false;   /* button reading on this pass */
	bool button_state_latched = false;  /* blink mode: toggled on each press */

	/* PD12 to PD15 (LEDs) as outputs: MODERx fields at bits 24, 26, 28, 30 = 01.
	 * Pin n's 2-bit field starts at bit 2n, so step i by 2. */
	for(uint32_t i = 24; i <= 30; i += 2){
		*GPIOD_MODER &= ~(3UL << i);  /* clear the field to 00 */
		*GPIOD_MODER |=  (1UL << i);  /* set it to 01 = output */
	}

	/* --- Loop forever --- */
	while(1){
		/* Read the button. When it reads pressed (IDR bit 0 = 1), the '&&' also
		 * runs timer_delay(50) before accepting it, which waits out contact bounce. */
		if((*GPIOA_IDR & (1UL << 0)) && timer_delay(50)){
			latest_button_state = true;
		}
		else{
			latest_button_state = false;
		}

		/* Rising edge: pressed now, released on the previous pass → toggle once */
		if(latest_button_state && !last_button_state){
			button_state_latched = !button_state_latched;
		}

		/* Blink mode on: one blink of all four LEDs per pass (500 ms on, 500 ms off) */
		if(button_state_latched){
			led_set();              /* all four on */
			timer_delay(500);
			led_reset();            /* all four off */
			timer_delay(500);
		}
		/* Blink mode off: keep all four LEDs off */
		else{
			led_reset();            /* all four off */
		}

		/* This pass's reading becomes the previous one for the next pass */
		last_button_state = latest_button_state;
	}
}
