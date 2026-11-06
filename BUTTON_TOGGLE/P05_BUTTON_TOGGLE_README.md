# P05: latched button toggle with simple debounce (bare-metal)

Each press of the blue user button toggles the green LED: press once and it turns on and stays on, press again and it turns off. No HAL, no CubeMX.

Where P04 was **momentary** (acts only while the button is held), P05 is **latched**: the LED keeps its state after the button is released.

## Hardware
- Board: STM32F407G-DISC1 (STM32F4-Discovery)
- Button: **B1 (USER)** on **PA0**, reads 1 when pressed
- LED: **PD12** green (LD4)
- Clock: 16 MHz internal HSI (the default after reset)

## How it works
**Setup, done once:** port A and port D clocks on (`RCC_AHB1ENR` bits 0 and 3), PA0 as input (MODER0 = `00`), PD12 as output (MODER12 = `01`).

**Loop, repeated forever:**
1. **Read the button** into `latest_button_state`. When it reads pressed, a 50 ms wait runs before the reading is accepted, to let contact bounce settle.
2. **Detect the press edge:** if the button is pressed now but was released on the previous pass (`latest && !last`), toggle `button_state_latched`.
3. **Drive the LED** from `button_state_latched`, using BSRR (set bit 12 for on, reset bit 28 for off).
4. **Remember the reading:** `last_button_state = latest_button_state`.

## Edge detection
Acting on the button's **level** (is it pressed?) would toggle the LED continuously while the button is held. Acting on the **edge** (released → pressed) toggles exactly once per press:

| Previous | Now | Action |
|---|---|---|
| 0 | 0 | none |
| 0 | 1 | **toggle** |
| 1 | 1 | none (still held) |
| 1 | 0 | none (released) |

## Bounce
A mechanical button's contacts flicker for a few milliseconds when they meet or separate, so one press can look like several to a fast loop. Each flicker would toggle the LED, so a press could seem to be ignored. The 50 ms wait after a pressed reading filters most of this out.

## Registers used
| Register | Address | Bits | Operation | Meaning |
|---|---|---|---|---|
| RCC_AHB1ENR | 0x4002 3830 | 0 (GPIOAEN), 3 (GPIODEN) | set | port A and port D clocks on |
| GPIOA_MODER | 0x4002 0000 | 1:0 (MODER0) | clear to 00 | PA0 = input |
| GPIOD_MODER | 0x4002 0C00 | 25:24 (MODER12) | clear, then set to 01 | PD12 = output |
| GPIOA_IDR | 0x4002 0010 | 0 (IDR0) | read, test with `&` | button pressed? |
| GPIOD_BSRR | 0x4002 0C18 | 12 (BS12) / 28 (BR12) | write 1 | LED on / off |

All values come from RM0090 and UM1472.

## Build and run
Open the project in STM32CubeIDE, build with the hammer icon, then click Run. Press the blue button to switch the green LED on; press again to switch it off.