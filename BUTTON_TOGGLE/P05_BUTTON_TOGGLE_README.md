# P05: latched button toggle with simple debounce (bare-metal)

Each press of the blue user button toggles a blink mode: press once and all four user LEDs start blinking together (500 ms on, 500 ms off), press again and they stop and stay off. No HAL, no CubeMX.

Where P04 was **momentary** (acts only while the button is held), P05 is **latched**: the mode stays on after the button is released, until the next press.

## Hardware
- Board: STM32F407G-DISC1 (STM32F4-Discovery)
- Button: **B1 (USER)** on **PA0**, reads 1 when pressed
- LEDs: **PD12** green (LD4), **PD13** orange (LD3), **PD14** red (LD5), **PD15** blue (LD6)
- Clock: 16 MHz internal HSI (the default after reset)

## How it works
**Setup, done once:**
1. Port A and port D clocks on (`RCC_AHB1ENR` bits 0 and 3).
2. PA0 as input (MODER0 = `00`), touching only its own bits.
3. PD12 to PD15 as outputs: a loop steps through the MODER fields at bits 24, 26, 28 and 30, clearing each to `00` and setting it to `01`.

**Loop, repeated forever:**
1. **Read the button** into `latest_button_state`. When it reads pressed, a 50 ms wait runs before the reading is accepted, to let contact bounce settle.
2. **Detect the press edge:** if the button is pressed now but was released on the previous pass (`latest && !last`), toggle `button_state_latched`.
3. **Blink mode on:** switch all four LEDs on (BSRR set bits 12 to 15), wait 500 ms, switch them off (reset bits 28 to 31), wait 500 ms.
4. **Blink mode off:** keep all four LEDs off.
5. **Remember the reading:** `last_button_state = latest_button_state`.

## Edge detection
Acting on the button's **level** (is it pressed?) would toggle the mode continuously while the button is held. Acting on the **edge** (released → pressed) toggles exactly once per press:

| Previous | Now | Action |
|---|---|---|
| 0 | 0 | none |
| 0 | 1 | **toggle** |
| 1 | 1 | none (still held) |
| 1 | 0 | none (released) |

## Bounce
A mechanical button's contacts flicker for a few milliseconds when they meet or separate, so one press can look like several to a fast loop. The 50 ms wait after a pressed reading filters most of this out.

## Known limitation
While blinking, each pass of the loop takes about 1 second, so the button is only checked once per blink. A quick tap in the middle of a blink can be missed; holding the button until the blink completes always works. A non-blocking version, which checks the button continuously while blinking, comes in P07.

## Registers used
| Register | Address | Bits | Operation | Meaning |
|---|---|---|---|---|
| RCC_AHB1ENR | 0x4002 3830 | 0 (GPIOAEN), 3 (GPIODEN) | set | port A and port D clocks on |
| GPIOA_MODER | 0x4002 0000 | 1:0 (MODER0) | clear to 00 | PA0 = input |
| GPIOD_MODER | 0x4002 0C00 | 31:24 (MODER12 to MODER15) | each field cleared, then set to 01 | PD12 to PD15 = outputs |
| GPIOA_IDR | 0x4002 0010 | 0 (IDR0) | read, test with `&` | button pressed? |
| GPIOD_BSRR | 0x4002 0C18 | 12 to 15 / 28 to 31 | write 1 | LEDs on / off |

All values come from RM0090 and UM1472.

## Build and run
Open the project in STM32CubeIDE, build with the hammer icon, then click Run. Press the blue button: all four LEDs start blinking. Press again: they stop.