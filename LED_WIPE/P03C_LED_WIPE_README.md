# P03c: LED wipe (bare-metal)

Switch the four user LEDs on the STM32F4-Discovery board on one by one until all are lit, then off one by one in the same order, 500 ms per step. Pins are switched with the **BSRR** register, and timing is a busy-wait delay. No HAL, no CubeMX.

## Hardware
- Board: STM32F407G-DISC1 (STM32F4-Discovery)
- LEDs: **PD12** green (LD4), **PD13** orange (LD3), **PD14** red (LD5), **PD15** blue (LD6)
- Clock: 16 MHz internal HSI (the default after reset)

## How it works
**Setup, done once:**
1. Switch on port D's clock: bit 3 (GPIODEN) in `RCC_AHB1ENR`, then read it back once so the clock is running.
2. Make PD12 to PD15 outputs. Each pin has a 2-bit field in `GPIOx_MODER` starting at bit 2n, so the fields for pins 12 to 15 start at bits 24, 26, 28 and 30. A loop clears each field to `00`, then sets it to `01`.

**Loop, repeated forever:**
1. **Fill:** for pins 12 to 15, write the pin's **set** bit to BSRR and wait 500 ms. The LEDs light up one after another.
2. **Empty:** for BSRR bits 28 to 31 (the **reset** bits for pins 12 to 15), write the bit and wait 500 ms. The LEDs go dark in the same order.

## Why BSRR
`GPIOx_BSRR` is split into two halves:
- **bits 0–15 (BSy):** writing 1 switches pin y **on**
- **bits 16–31 (BRy):** writing 1 switches pin y **off**, so pin n's reset bit is n + 16
- writing 0 to any bit does nothing

Switching a pin with ODR takes three steps (read, change, write back). BSRR does it in **one write**, and only touches the pins whose bits are 1, so a plain `=` is safe. BSRR is write-only, so it is never read or used with `|=`.

## Registers used
| Register | Address | Bits | Operation | Meaning |
|---|---|---|---|---|
| RCC_AHB1ENR | 0x4002 3830 | 3 (GPIODEN) | set | port D clock on |
| GPIOD_MODER | 0x4002 0C00 | 31:24 (MODER12 to MODER15) | each field cleared, then set to 01 | PD12 to PD15 = outputs |
| GPIOD_BSRR | 0x4002 0C18 | 12 to 15 (BS12 to BS15) | write 1 | one LED on |
| GPIOD_BSRR | 0x4002 0C18 | 28 to 31 (BR12 to BR15) | write 1 | one LED off |

All values come from RM0090 and UM1472.

## Build and run
Open the project in STM32CubeIDE, build with the hammer icon, then click Run. The LEDs fill up one by one, then empty one by one, 500 ms per step.