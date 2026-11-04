# P01: LED on (bare-metal)

Switch on the green LED (LD4) on the STM32F4-Discovery board by writing directly to the chip's registers: no HAL, no CubeMX.

## Hardware
- Board: STM32F407G-DISC1 (STM32F4-Discovery)
- LED: green LD4, connected to pin **PD12**
- Clock: 16 MHz internal HSI (the default after reset)

## How it works
Lighting an LED takes three steps, in this order:

1. **Switch on the port:** every GPIO port starts with its clock off. Setting bit 3 (GPIODEN) in `RCC_AHB1ENR` switches on port D.
2. **Make the pin an output:** each pin has a 2-bit mode field in `GPIOx_MODER`. For pin 12 that's bits 25:24, set to `01` (general-purpose output).
3. **Set the pin high:** each pin has one bit in `GPIOx_ODR`. Setting bit 12 drives PD12 to about 3 V, and the LED lights.

## Registers used
| Register | Address | Bits | Value | Meaning |
|---|---|---|---|---|
| RCC_AHB1ENR | 0x4002 3830 | 3 (GPIODEN) | 1 | port D clock on |
| GPIOD_MODER | 0x4002 0C00 | 25:24 (MODER12) | 01 | PD12 = output |
| GPIOD_ODR | 0x4002 0C14 | 12 (ODR12) | 1 | PD12 high |

All values come from RM0090 and UM1472.

## Build and run
Open the project in STM32CubeIDE, build with the hammer icon, then click Run. The green LED switches on and stays on.