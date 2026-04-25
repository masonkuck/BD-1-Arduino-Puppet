#include <Wire.h>
#include "config.h"
#include "ServoController.h"
#include "IdleMode.h"
#include "ControllerMode.h"

// =============================================================================
// BD-1 Main Sketch
// Mode switching lives here.
// =============================================================================

ServoController  servo;
IdleMode         idleMode(servo);
ControllerMode   controllerMode(servo);

enum class RobotMode {
    IDLE,
    CONTROLLER,
};

RobotMode currentMode = RobotMode::IDLE;

// --- Button debounce state ---
bool     _rawBtnState      = HIGH;  // Last raw reading
bool     _debouncedBtnState = HIGH; // Last stable reading after debounce
uint32_t _lastDebounceMs   = 0;

void setup() {
    Serial.begin(9600);
    randomSeed(analogRead(A1));

    pinMode(CTRL_BTN_MODE, INPUT_PULLUP);

    servo.begin();
    delay(1000); // Hold center before animation starts

    idleMode.begin();

    Serial.println("[BD-1] Ready.");
}

void loop() {
    // --- Mode switch button (debounced) ---
    bool reading = digitalRead(CTRL_BTN_MODE);
    if (reading != _rawBtnState) {
        _rawBtnState    = reading;
        _lastDebounceMs = millis();
    }
    if ((millis() - _lastDebounceMs) > CTRL_DEBOUNCE_MS) {
        if (_rawBtnState != _debouncedBtnState) {
            _debouncedBtnState = _rawBtnState;
            if (_debouncedBtnState == LOW) {
                // Button just pressed — toggle mode
                if (currentMode == RobotMode::IDLE) {
                    currentMode = RobotMode::CONTROLLER;
                    controllerMode.begin();
                } else {
                    currentMode = RobotMode::IDLE;
                    idleMode.begin();
                }
            }
        }
    }

    // --- Active mode update ---
    switch (currentMode) {
        case RobotMode::IDLE:
            idleMode.update();
            break;
        case RobotMode::CONTROLLER:
            controllerMode.update();
            break;
    }
}
