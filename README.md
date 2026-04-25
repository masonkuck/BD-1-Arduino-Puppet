# BD-1 Arduino Puppet Controller

Arduino firmware for a animatronic BD-1 droid ([MrBaddeley](https://www.patreon.com/c/mrbaddeley/posts) design), controlled by a handheld gimbal/knob puppet controller. Supports a real-time puppet mode and an autonomous idle animation mode.

> [!WARNING]
> While I have done a lot of testing, this project is largely still in progress. My main reason for pushing to GitHub is for version control, I probably wont be supporting this with requests however feel free to fork it and make changes. The sound from MrBaddeley's design is not present, because I have plans beyond it sitting on a desk so the sound is likely going to look very different.

## Hardware

| Component | Part |
|---|---|
| Microcontroller | Arduino Mega 2560 Pro Mini |
| Servo driver | PCA9685 16-channel PWM (I2C, address 0x40) |
| Controller input | 3-axis gimbal + 3 knob potentiometers, 4 buttons |

## Pinout

### I2C — Arduino Mega → PCA9685

| Mega Pin | PCA9685 Pin | Notes |
|---|---|---|
| Pin 20 SDA | SDA | |
| Pin 21 SCL | SCL | |
| 5V | VCC | |
| GND | GND | Common ground with servo PSU |

### PCA9685 — Servo Channels

| Channel | Servo | Notes |
|---|---|---|
| Ch 1 | Head Turn | |
| Ch 2 | Head Tilt L | |
| Ch 3 | Head Tilt R | Reversed |
| Ch 4 | Neck Nod | Reserved — not wired yet |
| Ch 5 | Eye Blink | |
| Ch 6 | Holoprojector | |
| Ch 7–13 | — | Unused |
| Ch 14 | Ear Left | |
| Ch 15 | Ear Right | Reversed |

V+ → Servo power supply positive (separate from Arduino 5V)  
GND → Servo power supply negative (common ground with Arduino)

### Analog — Potentiometers

Wiper to pin, ends to 5V and GND.

| Mega Pin | Pot | Notes |
|---|---|---|
| A1 | Eye open/close | Joystick top knob |
| A3 | Head Tilt | Joystick forward/back axis |
| A5 | Holoprojector | Knob |
| A7 | Ear Left | Knob |
| A9 | Ear Right | Knob |
| A11 | Head Turn | Joystick left/right axis |

### Digital — Buttons

Other side of button to GND, configured INPUT_PULLUP.

| Mega Pin | Button | Notes |
|---|---|---|
| D3 | Mode Switch | Toggles IDLE ↔ CONTROLLER |
| D5 | Sound 1 | Reserved — no sound yet |
| D7 | Ear Link | Toggle ears linked/independent |
| D9 | Sound 2 | Reserved — no sound yet |

## Modes

### Idle Mode
Autonomous animation — plays weighted-random keyframe sequences with eased interpolation. Eye blinks on an independent timer. Occasionally enters a "burst" of energetic sequences.

### Controller Mode
Real-time puppet control via potentiometers. All axes pass through EMA smoothing (input) and a spring-damper (output) for natural, lag-free movement. Eye is driven directly by the joystick top knob with no automatic blinking.

## Projects

### `BD1/`
Main firmware sketch. Open `BD1.ino` in the Arduino IDE.

**Dependencies:**
- [Adafruit PWM Servo Driver Library](https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library)

### `ServoTest/`
Standalone diagnostic sketch. One knob (A5) sweeps the selected servo through its configured range. D7/D9 step backward/forward through all servo channels. Serial monitor prints the current degree and raw ADC value at 100ms intervals.

## Configuration

All tunable constants are in `BD1/config.h` — servo channels, travel limits, pot calibration, smoothing factors, and blink timing. No other file should need changes for hardware adjustments.

Key tuning values:

| Constant | Default | Description |
|---|---|---|
| `CTRL_POT_ALPHA` | 0.15 | Input EMA smoothing (0=frozen, 1=raw) |
| `CTRL_SPRING` | 0.12 | Spring-damper pull strength |
| `CTRL_DAMPING` | 0.65 | Spring-damper damping (≈2√spring for no overshoot) |
| `CTRL_POT_DEADBAND` | 8 | ADC units of change required to update pot reading |
| `IDLE_BURST_CHANCE` | 15 | % chance of entering burst mode after each calm sequence |

Enable/disable controller Serial logging by commenting out `#define CTRL_DEBUG_LOG` in `config.h`.
