# P02: blink an LED (bare-metal)

Blink the green LED (LD4) on the STM32F4-Discovery board, 500 ms on and 500 ms off, using direct register access and a busy-wait delay. No HAL, no CubeMX.

## Hardware
- Board: STM32F407G-DISC1 (STM32F4-Discovery)
- LED: green LD4, connected to pin **PD12**
- Clock: 16 MHz internal HSI (the default after reset)

## How it works
**Setup, done once:**
1. Switch on port D's clock: bit 3 (GPIODEN) in `RCC_AHB1ENR`, then read the register back once so the clock is running before port D is used.
2. Make PD12 an output: clear MODER12 (bits 25:24) to `00`, then set it to `01`.

**Loop, repeated forever:**
1. LED on: set bit 12 in `GPIOx_ODR`.
2. Wait about 500 ms.
3. LED off: clear bit 12 in `GPIOx_ODR`.
4. Wait about 500 ms.

## Registers used
| Register | Address | Bits | Operation | Meaning |
|---|---|---|---|---|
| RCC_AHB1ENR | 0x4002 3830 | 3 (GPIODEN) | set | port D clock on |
| GPIOD_MODER | 0x4002 0C00 | 25:24 (MODER12) | clear, then set to 01 | PD12 = output |
| GPIOD_ODR | 0x4002 0C14 | 12 (ODR12) | set / clear | LED on / off |

All values come from RM0090 (reference manual, Rev __) and UM1472 (board user manual).

## The delay
`delay_ms()` is a **busy-wait**: the CPU counts in an empty loop to waste time. The loop counter is `volatile`, so the compiler cannot remove the loop.

The count of 2000 passes per millisecond was calibrated with a stopwatch at 16 MHz. It is approximate and depends on compiler settings. An accurate delay using the SysTick timer comes in P06.

## Build and run
Open the project in STM32CubeIDE, build with the hammer icon, then click Run. The green LED blinks: 500 ms on, 500 ms off.