#include "ServoController.h"
#include <Wire.h>

ServoController::ServoController() : _pwm(Adafruit_PWMServoDriver()) {}

void ServoController::begin() {
    Wire.begin();
    delay(100); // Bus stabilisation

    while (true) {
        Wire.beginTransmission(0x40);
        byte err = Wire.endTransmission();
        if (err == 0) {
            initHardware();
            centerAll();
            Serial.println("[ServoController] PCA9685 ready.");
            return;
        }
        Serial.print("[ServoController] PCA9685 not found (err ");
        Serial.print(err);
        Serial.println(") retrying...");
        delay(500);
    }
}

void ServoController::writeDegrees(uint8_t channel, int degrees,
                                    int minDeg, int maxDeg, bool reversed) {
    degrees = constrain(degrees, minDeg, maxDeg);
    if (reversed) {
        degrees = minDeg + (maxDeg - degrees);
    }
    int us = degreesToUs(degrees);
    writeMicroseconds(channel, us);
}

void ServoController::writeMicroseconds(uint8_t channel, int us) {
    us = constrain(us, SERVO_MIN_US, SERVO_MAX_US);
    _pwm.setPWM(channel, 0, usToPwmTicks(us));
}

void ServoController::centerAll() {
    for (int i = 0; i < NUM_IDLE_SERVOS; i++) {
        int center = IDLE_SERVO_RANGE[i][1];
        writeDegrees(IDLE_SERVO_CHANNELS[i], center,
                     IDLE_SERVO_RANGE[i][0], IDLE_SERVO_RANGE[i][2],
                     IDLE_SERVO_REVERSED[i]);
    }
}

// --- Private ---

void ServoController::initHardware() {
    _pwm.begin();
    _pwm.setOscillatorFrequency(OSCILLATOR_FREQ);
    _pwm.setPWMFreq(PWM_FREQ);
    delay(10);
}

int ServoController::degreesToUs(int degrees) {
    return map(degrees, 0, 180, SERVO_MIN_US, SERVO_MAX_US);
}

int ServoController::usToPwmTicks(int us) {
    return (int)((float)us / 1000000.0f * PWM_FREQ * 4096);
}