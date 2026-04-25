#pragma once

#include <Adafruit_PWMServoDriver.h>
#include "config.h"

// =============================================================================
// ServoController
// Wraps the PCA9685, handles degree→ticks conversion and channel reversal.
// =============================================================================
class ServoController {
public:
    ServoController();

    // Initialize PCA9685, retry until device is found
    void begin();

    // Write a position in degrees to a channel.
    // reversed: if true, mirrors the position within [minDeg, maxDeg]
    void writeDegrees(uint8_t channel, int degrees, int minDeg, int maxDeg, bool reversed);

    // Write a raw pulse width in microseconds to a channel
    void writeMicroseconds(uint8_t channel, int us);

    // Command all idle servos to their center positions
    void centerAll();

private:
    Adafruit_PWMServoDriver _pwm;

    int degreesToUs(int degrees);
    int usToPwmTicks(int us);
    void initHardware();
};