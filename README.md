# BD-1 Arduino Puppet Controller

Arduino firmware for a animatronic BD-1 droid (MrBaddeley design), controlled by a handheld gimbal/knob puppet controller. Supports a real-time puppet mode and an autonomous idle animation mode.

## Hardware

| Component | Part |
|---|---|
| Microcontroller | Arduino Mega 2560 Pro Mini |
| Servo driver | PCA9685 16-channel PWM (I2C, address 0x40) |
| Controller input | 3-axis gimbal + 3 knob potentiometers, 4 buttons |

### Servos

| PCA9685 Channel | Servo |
|---|---|
| Ch 1 | Head turn (left/right) |
| Ch 2 | Head tilt — left servo |
| Ch 3 | Head tilt — right servo (reversed) |
| Ch 4 | Neck nod (reserved) |
| Ch 5 | Eye blink |
| Ch 6 | Holoprojector pivot |
| Ch 14 | Ear left |
| Ch 15 | Ear right (reversed) |

### Controller Wiring

| Pin | Input | Function |
|---|---|---|
| A1 | Joystick top knob | Eye open/close |
| A3 | Joystick forward/back | Head tilt |
| A5 | Knob | Holoprojector |
| A7 | Knob | Ear left |
| A9 | Knob | Ear right |
| A11 | Joystick left/right | Head turn |
| D3 | Button | Toggle IDLE ↔ CONTROLLER mode |
| D5 | Button | Reserved |
| D7 | Button | Toggle ears linked/independent |
| D9 | Button | Reserved |

I2C to PCA9685 uses the Mega's dedicated pins: SDA=20, SCL=21.

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
