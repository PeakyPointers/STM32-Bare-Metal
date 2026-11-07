# P06: SysTick millisecond delay (bare-metal)

Replace the calibrated busy-wait loop with an accurate millisecond delay based on the Cortex-M4 **SysTick** timer. Demonstrated with the latched blink mode from P05: each press of the blue user button starts or stops all four user LEDs blinking together, 500 ms on and 500 ms off. No HAL, no CubeMX.

## Hardware
- Board: STM32F407G-DISC1 (STM32F4-Discovery)
- Button: **B1 (USER)** on **PA0**, reads 1 when pressed
- LEDs: **PD12** green (LD4), **PD13** orange (LD3), **PD14** red (LD5), **PD15** blue (LD6)
- Clock: 16 MHz internal HSI (the default after reset)

## SysTick
SysTick is a 24-bit down-counter built into the ARM Cortex-M4 core (documented in PM0214, not RM0090). It counts down from the value in `STK_LOAD` to 0 once per clock tick, then reloads and sets `COUNTFLAG`.

The counter takes one tick more than the loaded value, so for a wrap every N ticks, LOAD = N − 1:

| | Value |
|---|---|
| Clock | 16 MHz = 16,000,000 ticks/s |
| Ticks per 1 ms | 16,000 |
| LOAD | 16,000 − 1 = **15,999** (`0x3E7F`) |

**Setup (`systick_init`):** set LOAD to 15,999, clear VAL (which also clears COUNTFLAG), then set CTRL to `0x05`: CLKSOURCE = processor clock, ENABLE = on, TICKINT = 0 (no interrupt, the flag is polled).

**Delay:** `timer_1ms()` waits until COUNTFLAG reads 1 (reading CTRL clears it again), so each call lasts exactly one 1 ms wrap. `timer_delay(ms)` calls it `ms` times.

Unlike the old busy-wait loop, the delay no longer depends on compiler settings or calibration. Its accuracy is that of the 16 MHz internal oscillator (about 1%).

## How the application works
**Setup:** SysTick 1 ms timebase; port A and port D clocks on; PA0 as input; PD12 to PD15 as outputs.

**Loop:**
1. Read the button; when pressed, wait 50 ms (debounce) before accepting the reading.
2. On the rising edge (pressed now, released before), toggle the blink mode.
3. Blink mode on: all four LEDs on, 500 ms, all off, 500 ms. Blink mode off: LEDs off.

`timer_delay()` is a blocking delay: while it waits, nothing else runs, so in blink mode the button is only checked once per blink. Non-blocking timing comes in P07.

## Registers used
| Register | Address | Bits | Operation | Meaning |
|---|---|---|---|---|
| STK_CTRL | 0xE000 E010 | 0 (ENABLE), 2 (CLKSOURCE) | write 0x05 | SysTick on, processor clock |
| STK_CTRL | 0xE000 E010 | 16 (COUNTFLAG) | read | 1 ms has passed |
| STK_LOAD | 0xE000 E014 | 23:0 | write 15,999 | 1 ms reload value |
| STK_VAL | 0xE000 E018 | 23:0 | write 0 | clear current count |
| RCC_AHB1ENR | 0x4002 3830 | 0 (GPIOAEN), 3 (GPIODEN) | set | port A and port D clocks on |
| GPIOA_MODER | 0x4002 0000 | 1:0 (MODER0) | clear to 00 | PA0 = input |
| GPIOD_MODER | 0x4002 0C00 | 31:24 (MODER12 to MODER15) | each field cleared, then set to 01 | PD12 to PD15 = outputs |
| GPIOA_IDR | 0x4002 0010 | 0 (IDR0) | read, test with `&` | button pressed? |
| GPIOD_BSRR | 0x4002 0C18 | 12 to 15 / 28 to 31 | write 1 | LEDs on / off |

All values come from RM0090, PM0214 and UM1472.

## Build and run
Open the project in STM32CubeIDE, build with the hammer icon, then click Run. Press the blue button: all four LEDs blink, 500 ms on and 500 ms off. 10 full blinks take 10 seconds.