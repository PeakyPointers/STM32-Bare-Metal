# P03a: flash all four LEDs (bare-metal)

Switch all four user LEDs on the STM32F4-Discovery board on together, then off together, every 500 ms, using direct register access and a busy-wait delay. No HAL, no CubeMX.

A stepping stone towards P03b, where the LEDs light one at a time in a circle.

## Hardware
- Board: STM32F407G-DISC1 (STM32F4-Discovery)
- LEDs: **PD12** green (LD4), **PD13** orange (LD3), **PD14** red (LD5), **PD15** blue (LD6)
- Clock: 16 MHz internal HSI (the default after reset)

## How it works
**Setup, done once:**
1. Switch on port D's clock: bit 3 (GPIODEN) in `RCC_AHB1ENR`, then read it back once so the clock is running.
2. Make PD12 to PD15 outputs. Each pin has a 2-bit field in `GPIOx_MODER` starting at bit 2n, so for pins 12 to 15 the fields start at bits 24, 26, 28 and 30. A loop steps through them, clearing each field to `00` and then setting it to `01`.

**Loop, repeated forever:**
1. All on: set bits 12 to 15 in `GPIOx_ODR`.
2. Wait about 500 ms.
3. All off: clear bits 12 to 15 in `GPIOx_ODR`.
4. Wait about 500 ms.

## Registers used
| Register | Address | Bits | Operation | Meaning |
|---|---|---|---|---|
| RCC_AHB1ENR | 0x4002 3830 | 3 (GPIODEN) | set | port D clock on |
| GPIOD_MODER | 0x4002 0C00 | 31:24 (MODER12 to MODER15) | each field cleared, then set to 01 | PD12 to PD15 = outputs |
| GPIOD_ODR | 0x4002 0C14 | 15:12 | set / clear | LEDs on / off |

All values come from RM0090 (reference manual, Rev __) and UM1472 (board user manual).

## Build and run
Open the project in STM32CubeIDE, build with the hammer icon, then click Run. All four LEDs flash together: 500 ms on, 500 ms off.