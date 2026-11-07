# STM32-Bare-Metal

Work in progress: bare-metal STM32F407 projects in C, written from the reference manual with no HAL or CubeMX.

Every register address and bit in this repo comes from ST's documentation, not from generated code. Each project has its own README explaining how it works and which registers it uses.

## Hardware
- **Board:** STM32F407G-DISC1 (STM32F4-Discovery)
- **MCU:** STM32F407VGT6 (Arm Cortex-M4F, 168 MHz max, 1 MB flash, 192 KB RAM)
- **On-board:** 4 user LEDs (PD12 to PD15), user button (PA0), ST-LINK/V2-A programmer and debugger
- **Clock:** 16 MHz internal HSI (default after reset)

## Tools
- **IDE / compiler:** STM32CubeIDE (GCC for Arm), Empty projects: no HAL, no generated code
- **Programming / debugging:** on-board ST-LINK

## References
| Document | Covers |
|---|---|
| **RM0090** | STM32F405/407 reference manual: GPIO, RCC and all other peripherals |
| **PM0214** | STM32 Cortex-M4 programming manual: SysTick, NVIC, FPU |
| **UM1472** | STM32F4-Discovery board user manual: what is connected to which pin |

## Projects

| # | Project | What it does | Concepts |
|---|---|---|---|
| P01 | `P01_led_on` | Green LED on | RCC clock enable, MODER, ODR |
| P02 | `P02_blink` | Green LED blinks | busy-wait delay, `volatile` |
| P03a | `P03a_led_flash_all` | All four LEDs flash together | 2-bit MODER fields in a loop |
| P03b | `P03b_led_circle` | One LED at a time, round the ring | one function for many pins |
| P03c | `P03c_led_wipe` | LEDs fill up, then empty | BSRR single-write set/reset |
| P04 | `P04_button_led` | LED wipe while the button is held | input pin, IDR, bit test |
| P05 | `P05_button_toggle` | Each press toggles a blink mode | edge detection, debounce |
| P06 | `P06_systick_delay` | Accurate millisecond delay | SysTick timer (Cortex-M4 core) |

```