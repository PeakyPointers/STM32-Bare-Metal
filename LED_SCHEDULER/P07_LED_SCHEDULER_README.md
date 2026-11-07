# P07: LED scheduler (bare-metal)

A cooperative, time-triggered scheduler driven by a 1 ms SysTick tick. Tasks run at fixed rates (10, 50, 100, 250, 500 and 1000 ms) with no blocking delays. The blue user button toggles a blink mode in which all four user LEDs blink together, 500 ms on and 500 ms off. No HAL, no CubeMX.

This is the same structure used in many embedded and automotive systems: fixed-rate tasks ("rasters") called by a scheduler, instead of a main loop full of delays.

## Hardware
- Board: STM32F407G-DISC1 (STM32F4-Discovery)
- Button: **B1 (USER)** on **PA0**, reads 1 when pressed
- LEDs: **PD12** green (LD4), **PD13** orange (LD3), **PD14** red (LD5), **PD15** blue (LD6)
- Clock: 16 MHz internal HSI (the default after reset)

## How it works

### The scheduler
`scheduler()` loops forever:
1. Wait for the next 1 ms SysTick wrap (`timer_1ms()` polls COUNTFLAG). This is the only waiting anywhere in the program.
2. Count the tick.
3. Run every task whose period divides the tick count:

| Task | Period | Job |
|---|---|---|
| `task_10ms` | 10 ms | unused |
| `task_50ms` | 50 ms | sample the button, toggle blink mode on each press |
| `task_100ms` | 100 ms | unused |
| `task_250ms` | 250 ms | unused |
| `task_500ms` | 500 ms | flip all four LEDs while blink mode is on |
| `task_1000ms` | 1000 ms | unused |

The tick count restarts at 1000, which works because 1000 is a multiple of every task period.

### Rules for tasks
- A task does a short piece of work and **returns**. No loops that wait and no delays, or every other task runs late.
- All tasks due in the same tick must finish within 1 ms. COUNTFLAG only records one wrap, so a longer pass would lose ticks.
- State that must survive between runs (button readings, blink mode, LED state) is stored in file-level variables.

### Button and debounce
`task_50ms` reads PA0 every 50 ms. Contact bounce lasts a few milliseconds, so sampling every 50 ms never sees more than one change per press: the slow sample rate is itself the debounce. A press is detected on the rising edge (pressed now, released on the previous sample) and toggles `button_state_latched`.

### Blink
While blink mode is on, each run of `task_500ms` either switches the LEDs on or off, depending on `led_state`, then flips `led_state`. One run every 500 ms gives 500 ms on and 500 ms off. When blink mode is off, the LEDs are kept off.

## Registers used
| Register | Address | Bits | Operation | Meaning |
|---|---|---|---|---|
| STK_CTRL | 0xE000 E010 | 0 (ENABLE), 2 (CLKSOURCE) | write 0x05 | SysTick on, processor clock |
| STK_CTRL | 0xE000 E010 | 16 (COUNTFLAG) | read | 1 ms tick |
| STK_LOAD | 0xE000 E014 | 23:0 | write 15,999 | 1 ms reload value |
| STK_VAL | 0xE000 E018 | 23:0 | write 0 | clear current count |
| RCC_AHB1ENR | 0x4002 3830 | 0 (GPIOAEN), 3 (GPIODEN) | set | port A and port D clocks on |
| GPIOA_MODER | 0x4002 0000 | 1:0 (MODER0) | clear to 00 | PA0 = input |
| GPIOD_MODER | 0x4002 0C00 | 31:24 (MODER12 to MODER15) | each field cleared, then set to 01 | PD12 to PD15 = outputs |
| GPIOA_IDR | 0x4002 0010 | 0 (IDR0) | read, test with `&` | button pressed? |
| GPIOD_BSRR | 0x4002 0C18 | 12 to 15 / 28 to 31 | write 1 | LEDs on / off |

All values come from RM0090, PM0214 and UM1472.

## Build and run
Open the project in STM32CubeIDE, build with the hammer icon, then click Run. Press the blue button: all four LEDs blink together. Press again: they stop. The button responds at any moment, because nothing in the program blocks.