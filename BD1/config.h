#pragma once

// =============================================================================
// BD-1 Configuration
// All servo channels, ranges, and tuning constants live here.
// Nothing else should need to change when adjusting hardware layout or feel.
// =============================================================================


// =============================================================================
// PCA9685 PWM Driver
// =============================================================================
#define PWM_FREQ            50          // Servo refresh rate (Hz) — standard is 50
#define OSCILLATOR_FREQ     25000000    // PCA9685 crystal frequency (Hz)
                                        // Increase if servos run slow; decrease if fast


// =============================================================================
// Servo Pulse Range
// Defines the microsecond range sent to servos.
// 500–2500 µs covers most hobby servos; narrow to 1000–2000 if a servo twitches
// at the extremes. Degrees are always mapped across the full 0–180° span so that
// 90° = 1500 µs = physical center, regardless of each servo's min/max limits.
// =============================================================================
#define SERVO_MIN_US        500
#define SERVO_MAX_US        2500


// =============================================================================
// Head Servo Channel Assignments (PCA9685, 0-indexed)
// Physical slot on the PCA9685 board, counting from 0.
// =============================================================================
#define CH_HEAD_TURN        1   // Head rotation left/right (Z axis)
#define CH_HEAD_TILT_L      2   // Head tilt — left servo
#define CH_HEAD_TILT_R      3   // Head tilt — right servo (reversed, mirrored pair)
#define CH_NECK_NOD         4   // Neck nod — reserved, not used in animations yet
#define CH_EYE_BLINK        5   // Eye blink — reserved, eyes not assembled yet
#define CH_HOLO             6   // Holoprojector pivot
#define CH_EAR_L            14  // Left ear
#define CH_EAR_R            15  // Right ear (reversed, mirrored pair)


// =============================================================================
// Idle Servo Index Map
// IdleMode manages 6 servos (excludes reserved channels: neck nod, eye blink).
// IDX_ constants index into the arrays below — keep all arrays in this order.
// =============================================================================
#define NUM_IDLE_SERVOS     7

#define IDX_HEAD_TURN       0
#define IDX_HEAD_TILT_L     1
#define IDX_HEAD_TILT_R     2
#define IDX_HOLO            3
#define IDX_EAR_L           4
#define IDX_EAR_R           5
#define IDX_EYE_BLINK       6   // Independently controlled — not in keyframes


// =============================================================================
// Reversal Flags
// Set true for servos mounted in reverse (mirrors movement within the range).
// Must stay in IDX_ order.
// =============================================================================
const bool IDLE_SERVO_REVERSED[NUM_IDLE_SERVOS] = {
    true,   // HEAD_TURN
    false,  // HEAD_TILT_L
    true,   // HEAD_TILT_R  — mirrored pair, physically reversed
    false,  // HOLO
    false,  // EAR_L
    true,   // EAR_R        — mirrored pair, physically reversed
    false,  // EYE_BLINK
};


// =============================================================================
// Servo Travel Limits  { min°, center°, max° }
// These are software clamps — the servo will never be commanded outside this
// range regardless of what an animation requests. Degrees map to microseconds
// against the full 0–180° span, so 90° always = 1500 µs = physical center.
//
// Tune min/max after physical assembly to prevent over-travel.
// Center should almost always stay 90 (= 1500 µs).
// Must stay in IDX_ order.
// =============================================================================
const int IDLE_SERVO_RANGE[NUM_IDLE_SERVOS][3] = {
    {  50, 90, 130 },   // HEAD_TURN   — 40° each side; expand once tested
    {  50, 90, 110 },   // HEAD_TILT_L — asymmetric: more tilt range above center
    {  50, 90, 110 },   // HEAD_TILT_R — mirrors L (reversal handled in code)
    {  75, 90, 105 },   // HOLO        — small sweep
    {   0, 60, 120 },   // EAR_L       — full range: 0=closed, 120=open
    {   0, 60, 120 },   // EAR_R       — mirrors L (reversal handled in code)
    {  0, 130,  130 },   // EYE_BLINK   — min=closed, center/max=open; tune after assembly
};


// =============================================================================
// PCA9685 Channel List for Idle Servos
// Maps IDX_ positions to physical PCA9685 channels.
// Must stay in IDX_ order.
// =============================================================================
const uint8_t IDLE_SERVO_CHANNELS[NUM_IDLE_SERVOS] = {
    CH_HEAD_TURN,
    CH_HEAD_TILT_L,
    CH_HEAD_TILT_R,
    CH_HOLO,
    CH_EAR_L,
    CH_EAR_R,
    CH_EYE_BLINK,
};


// =============================================================================
// Controller Mode — Hardware Pin Assignments
// Running on Arduino Mega 2560 Pro Mini. I2C uses dedicated pins 20/21,
// so A0–A5 are all free for analog input.
// =============================================================================

// Potentiometers (analog in, 0–1023)
#define CTRL_POT_HEAD_TURN  A11 // Joystick left/right axis — head turn
#define CTRL_POT_HEAD_TILT  A3  // Joystick forward/back axis — head tilt
#define CTRL_POT_HOLO       A5  // Gimbal axis 3 — holoprojector
#define CTRL_POT_EAR_L      A7  // Knob 1 — left ear
#define CTRL_POT_EAR_R      A9  // Knob 2 — right ear
#define CTRL_POT_SPARE      A1  // Joystick top knob — reserved

// Buttons (digital in, active LOW with INPUT_PULLUP)
#define CTRL_BTN_MODE       3   // Toggle IDLE ↔ CONTROLLER
#define CTRL_BTN_BLINK      5   // Manual blink — press=blink, hold=close, release=open
#define CTRL_BTN_S2         7   // Reserved — sound 2
#define CTRL_BTN_S3         9   // Reserved — sound 3

// Tuning
#define CTRL_POT_DEADBAND   8       // ADC units — ignore changes smaller than this (applied after smoothing)
#define CTRL_POT_ALPHA      0.15f   // EMA smoothing factor — lower = smoother but more lag (range: 0.05–0.5)
#define CTRL_SPRING         0.12f   // Spring constant — how hard it pulls toward target (higher = snappier)
#define CTRL_DAMPING        0.65f   // Damping — resists velocity to prevent overshoot (critical ≈ 2√spring)
#define CTRL_DEBOUNCE_MS    50      // Button debounce window (ms)

// Debug — comment out to disable controller logging in Serial Monitor
#define CTRL_DEBUG_LOG
#define CTRL_DEBUG_INTERVAL_MS  100

// Pot calibration — actual ADC min/max at mechanical limits (read from ControllerTest)
// Gimbal pots have restricted throw; knob pots run nearly full range
#define CTRL_POT_HEAD_TURN_MIN  283
#define CTRL_POT_HEAD_TURN_MAX  515
#define CTRL_POT_HEAD_TILT_MIN  200
#define CTRL_POT_HEAD_TILT_MAX  540
#define CTRL_POT_HOLO_MIN       0
#define CTRL_POT_HOLO_MAX       720
#define CTRL_POT_EAR_L_MIN      0
#define CTRL_POT_EAR_L_MAX      700
#define CTRL_POT_EAR_R_MIN      0
#define CTRL_POT_EAR_R_MAX      700
#define CTRL_POT_SPARE_MIN      0
#define CTRL_POT_SPARE_MAX      700

// =============================================================================
// Idle Mode Behaviour Tuning
// =============================================================================
#define IDLE_BLINK_MIN_MS       10000    // Minimum ms between eye blinks
#define IDLE_BLINK_MAX_MS       15000    // Maximum ms between eye blinks
#define BLINK_CLOSE_MS          100     // Time to close eyelid (ms)
#define BLINK_HOLD_MS           80      // Time to hold closed (ms)
#define BLINK_OPEN_MS           150     // Time to open eyelid (ms)
#define IDLE_BURST_CHANCE       15      // % chance of entering burst after each calm sequence
#define IDLE_BURST_SEQ_COUNT    3       // How many sequences play during a burst
