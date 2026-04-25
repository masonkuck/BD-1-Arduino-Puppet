#pragma once

#include "ServoController.h"
#include "config.h"

// =============================================================================
// ControllerMode
// Puppet control — potentiometers map directly to servos in real time.
// Eye blink continues on its independent timer.
// Mode switch button is handled in BD1.ino.
// =============================================================================
class ControllerMode {
public:
    ControllerMode(ServoController& sc);

    void begin();   // Center servos, seed pot state, print ready message
    void update();  // Call every loop() — non-blocking

private:
    ServoController& _sc;

    // Pot state — smoothed then deadbanded
    // Order: HEAD_TURN, HEAD_TILT, HOLO, EAR_L, EAR_R, SPARE
    float _smoothed[6];  // EMA accumulator (float for precision)
    int   _lastRaw[6];   // Last accepted value after deadband

    // Spring-damper — one axis per independent control input
    // Order: HEAD_TURN, HEAD_TILT, HOLO, EAR_L, EAR_R, EYE
    static const int NUM_AXES = 6;
    float _pos[NUM_AXES];  // Current smoothed output position (degrees)
    float _vel[NUM_AXES];  // Current velocity (degrees per update)

    // Step the spring-damper for one axis toward target; clamps output to [minDeg, maxDeg]
    float updateSpring(int idx, float target, int minDeg, int maxDeg);

    // Ear link toggle (CTRL_BTN_S2)
    bool     _earsLinked;
    bool     _earsBtnRaw;
    bool     _earsBtnDebounced;
    uint32_t _earsBtnMs;

    // Read a pot with deadband; updates _lastRaw[idx] if change is large enough
    int  readPot(uint8_t pin, int idx);

    // Map a raw ADC reading to a degree, using calibrated pot min/max
    float potToDeg(int raw, int potMin, int potMax, int minDeg, int maxDeg);

    void readAndWriteServos();
    void updateEye();
};
