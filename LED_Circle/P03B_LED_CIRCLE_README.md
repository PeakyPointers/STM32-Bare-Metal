# P03b: LED circle (bare-metal)

Light the four user LEDs on the STM32F4-Discovery board one at a time, moving round the ring with no gap. Each LED is on for 500 ms, then the light jumps straight to the next one. Direct register access and a busy-wait delay. No HAL, no CubeMX.

## Hardware
- Board: STM32F407G-DISC1 (STM32F4-Discovery)
- LEDs: **PD12** green (LD4), **PD13** orange (LD3), **PD14** red (LD5), **PD15** blue (LD6), arranged in a ring on the board
- Clock: 16 MHz internal HSI (the default after reset)

## How it works
**Setup, done once:**
1. Switch on port D's clock: bit 3 (GPIODEN) in `RCC_AHB1ENR`, then read it back once so the clock is running.
2. Make PD12 to PD15 outputs. Each pin has a 2-bit field in `GPIOx_MODER` starting at bit 2n, so the fields for pins 12 to 15 start at bits 24, 26, 28 and 30. A loop clears each field to `00`, then sets it to `01`.

**Loop, repeated forever:** for each pin from 12 to 15:
1. Switch this LED on.
2. Wait 500 ms.
3. Switch this LED off. The loop moves straight on to the next pin, so the next LED lights immediately.

Only one LED is ever on at a time, so the light appears to travel smoothly round the ring.

`led_set(pin)` and `led_reset(pin)` take a pin number and change only that pin's bit in ODR, so the same two functions work for all four LEDs.

## Registers used
| Register | Address | Bits | Operation | Meaning |
|---|---|---|---|---|
| RCC_AHB1ENR | 0x4002 3830 | 3 (GPIODEN) | set | port D clock on |
| GPIOD_MODER | 0x4002 0C00 | 31:24 (MODER12 to MODER15) | each field cleared, then set to 01 | PD12 to PD15 = outputs |
| GPIOD_ODR | 0x4002 0C14 | 12 to 15, one at a time | set / clear | one LED on / off |

All values come from RM0090 and UM1472.

## Build and run
Open the project in STM32CubeIDE, build with the hammer icon, then click Run. The light travels round the ring, one LED at a time, 500 ms each.