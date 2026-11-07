/**
 * P07: LED scheduler (bare-metal)
 *
 * A cooperative, time-triggered scheduler driven by a 1 ms SysTick tick runs
 * tasks at fixed rates (10, 50, 100, 250, 500 and 1000 ms), with no blocking
 * delays. The 50 ms task samples the blue user button (B1); sampling every
 * 50 ms is slower than contact bounce, so it also debounces the button. Each
 * press toggles blink mode, and the 500 ms task flips all four user LEDs on
 * each run while blink mode is on (500 ms on, 500 ms off). No HAL, no CubeMX.
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

/* Shared state: declared at file level so it keeps its value between task calls
 * and can be used by more than one task */
bool last_button_state = false;     /* button reading on the previous 50 ms sample */
bool latest_button_state = false;   /* button reading on this 50 ms sample */
bool button_state_latched = false;  /* blink mode: toggled on each press */
bool led_state = false;             /* LEDs currently on (true) or off (false) */

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
 * Reading STK_CTRL clears COUNTFLAG, ready for the next millisecond.
 * In the scheduler this is the idle time between ticks. */
bool timer_1ms(){
	while(!(*STK_CTRL & (1UL << 16))){
		/* wait */
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

/* ---- Tasks: each one does a short piece of work and returns ---- */

/* 10 ms task: unused */
static void task_10ms(void){
}

/* 50 ms task: sample the button and toggle blink mode on each press.
 * Sampling every 50 ms is slower than contact bounce, so this also debounces. */
static void task_50ms(void){

	if((*GPIOA_IDR & (1UL << 0))){
		latest_button_state = true;
	}
	else{
		latest_button_state = false;
	}

	/* Rising edge: pressed now, released on the previous sample → toggle once */
	if(latest_button_state && !last_button_state){
		button_state_latched = !button_state_latched;
	}

	/* This sample becomes the previous one for the next run */
	last_button_state = latest_button_state;
}

/* 100 ms task: unused */
static void task_100ms(void){
}

/* 250 ms task: unused */
static void task_250ms(void){
}

/* 500 ms task: in blink mode, flip all four LEDs once per run (500 ms on, 500 ms off) */
static void task_500ms(void){
	if(button_state_latched){
		if(led_state){
			led_reset();            /* all four off */
		}
		else{
			led_set();              /* all four on */
		}
		led_state = !led_state;
	}
	/* Blink mode off: keep all four LEDs off */
	else{
		led_reset();                /* all four off */
		led_state = false;
	}
}

/* 1000 ms task: unused */
static void task_1000ms(void){
}

/* Cooperative scheduler: wait for each 1 ms tick, then run every task that is due.
 * A task is due when the tick count is a multiple of its period. */
void scheduler()
{
    uint32_t i = 0;
    while(1)
    {
        // Count this tick
        i++;

        // Wait for the next 1 ms SysTick wrap
        timer_1ms();

        // 10 ms task: every 10 ticks
        if(i % 10 == 0)
        {
            task_10ms();
        }

        // 50 ms task: every 50 ticks
        if(i % 50 == 0)
		{
			task_50ms();
		}

        // 100 ms task: every 100 ticks
        if(i % 100 == 0)
        {
            task_100ms();
        }

        // 250 ms task: every 250 ticks
        if(i % 250 == 0)
		{
			task_250ms();
		}

        // 500 ms task: every 500 ticks
        if(i % 500 == 0)
		{
			task_500ms();
		}

        // 1000 ms task: every 1000 ticks, then restart the count.
        // Safe because 1000 is a multiple of every period above.
        if(i % 1000 == 0)
        {
            task_1000ms();
            i = 0;
        }
    }
}

/* Blocking delay of ms milliseconds: wait for ms SysTick wraps, then return true.
 * Not used by the scheduler. */
bool timer_delay(uint32_t ms){
	for(uint32_t i = 0; i < ms; i++){
		timer_1ms();
	}
	return 1;
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

	/* PD12 to PD15 (LEDs) as outputs: MODERx fields at bits 24, 26, 28, 30 = 01.
	 * Pin n's 2-bit field starts at bit 2n, so step i by 2. */
	for(uint32_t i = 24; i <= 30; i += 2){
		*GPIOD_MODER &= ~(3UL << i);  /* clear the field to 00 */
		*GPIOD_MODER |=  (1UL << i);  /* set it to 01 = output */
	}

	/* --- Run the scheduler forever --- */
	scheduler();
}
