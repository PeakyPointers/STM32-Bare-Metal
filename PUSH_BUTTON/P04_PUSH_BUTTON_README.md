# P04: button-triggered LED wipe, momentary (bare-metal)

While the blue user button is held, the four user LEDs on the STM32F4-Discovery board run a wipe: on one by one, then off one by one, 500 ms per step. When the button is released, all LEDs go off. The button is read from `GPIOA_IDR`; the LEDs are switched with BSRR. No HAL, no CubeMX.

This is **momentary** behaviour: the code checks whether the button is pressed right now. Latched behaviour (one press toggles) comes in P05.

## Hardware
- Board: STM32F407G-DISC1 (STM32F4-Discovery)
- Button: **B1 (USER)** on **PA0**. Pressing it connects PA0 to 3 V, so it reads 1. Released, it reads 0.
- LEDs: **PD12** green (LD4), **PD13** orange (LD3), **PD14** red (LD5), **PD15** blue (LD6)
- Clock: 16 MHz internal HSI (the default after reset)

## How it works
**Setup, done once:**
1. Switch on the clocks for **port A** (bit 0, GPIOAEN) and **port D** (bit 3, GPIODEN) in `RCC_AHB1ENR`, then read it back once so they're running.
2. Make **PA0 an input:** clear MODER0 (bits 1:0) to `00`. Only PA0's bits are touched, because PA13 and PA14 on the same port are the ST-LINK's debug pins.
3. Make **PD12 to PD15 outputs:** each 2-bit MODER field (bits 24, 26, 28, 30) is cleared to `00`, then set to `01`.

**Loop, repeated forever:**
- **Button held** (`GPIOA_IDR` bit 0 = 1): run one wipe. Fill PD12 → PD15, then empty PD12 → PD15, 500 ms per step.
- **Button released:** switch all four LEDs off.

The button is only checked between wipes, because the busy-wait delay keeps the CPU occupied during a wipe.

## Registers used
| Register | Address | Bits | Operation | Meaning |
|---|---|---|---|---|
| RCC_AHB1ENR | 0x4002 3830 | 0 (GPIOAEN), 3 (GPIODEN) | set | port A and port D clocks on |
| GPIOA_MODER | 0x4002 0000 | 1:0 (MODER0) | clear to 00 | PA0 = input |
| GPIOD_MODER | 0x4002 0C00 | 31:24 (MODER12 to MODER15) | each field cleared, then set to 01 | PD12 to PD15 = outputs |
| GPIOA_IDR | 0x4002 0010 | 0 (IDR0) | read, test with `&` | button pressed? |
| GPIOD_BSRR | 0x4002 0C18 | 12 to 15 / 28 to 31 | write 1 | LED on / off |

All values come from RM0090 and UM1472.

## Build and run
Open the project in STM32CubeIDE, build with the hammer icon, then click Run. Hold the blue button: the LEDs fill up and empty, again and again. Release it: the LEDs go off once the current wipe finishes.