#include "ControllerMode.h"

ControllerMode::ControllerMode(ServoController& sc) : _sc(sc) {}

void ControllerMode::begin() {
    // Seed pot state from current readings so servos don't jump on entry
    _lastRaw[0] = analogRead(CTRL_POT_HEAD_TURN);
    _lastRaw[1] = analogRead(CTRL_POT_HEAD_TILT);
    _lastRaw[2] = analogRead(CTRL_POT_HOLO);
    _lastRaw[3] = analogRead(CTRL_POT_EAR_L);
    _lastRaw[4] = analogRead(CTRL_POT_EAR_R);
    _lastRaw[5] = analogRead(CTRL_POT_SPARE);

    for (int i = 0; i < 6; i++) _smoothed[i] = (float)_lastRaw[i];

    // Seed spring positions at center so servos don't snap on entry
    for (int i = 0; i < NUM_AXES; i++) {
        _vel[i] = 0.0f;
    }
    _pos[0] = IDLE_SERVO_RANGE[IDX_HEAD_TURN][1];
    _pos[1] = IDLE_SERVO_RANGE[IDX_HEAD_TILT_L][1];
    _pos[2] = IDLE_SERVO_RANGE[IDX_HOLO][1];
    _pos[3] = IDLE_SERVO_RANGE[IDX_EAR_L][1];
    _pos[4] = IDLE_SERVO_RANGE[IDX_EAR_R][1];
    _pos[5] = IDLE_SERVO_RANGE[IDX_EYE_BLINK][2]; // start open

    _earsLinked       = false;
    _earsBtnRaw       = HIGH;
    _earsBtnDebounced = HIGH;
    _earsBtnMs        = 0;
    pinMode(CTRL_BTN_S2, INPUT_PULLUP);

    _sc.centerAll();

    Serial.println("[ControllerMode] Started.");
}

void ControllerMode::update() {
    // Ear link toggle
    bool reading = digitalRead(CTRL_BTN_S2);
    if (reading != _earsBtnRaw) { _earsBtnRaw = reading; _earsBtnMs = millis(); }
    if ((millis() - _earsBtnMs) > CTRL_DEBOUNCE_MS && _earsBtnRaw != _earsBtnDebounced) {
        _earsBtnDebounced = _earsBtnRaw;
        if (_earsBtnDebounced == LOW) {
            _earsLinked = !_earsLinked;
            Serial.print("[ControllerMode] Ears: ");
            Serial.println(_earsLinked ? "LINKED" : "INDEPENDENT");
        }
    }

    readAndWriteServos();
    updateEye();

#ifdef CTRL_DEBUG_LOG
    static uint32_t _lastLogMs = 0;
    if (millis() - _lastLogMs >= CTRL_DEBUG_INTERVAL_MS) {
        _lastLogMs = millis();
        char buf[80];
        snprintf(buf, sizeof(buf),
            "%-5d %-5d %-5d %-5d %-5d %-5d | %-5d %-5d %-5d %-5d",
            _lastRaw[0], _lastRaw[1], _lastRaw[2],
            _lastRaw[3], _lastRaw[4], _lastRaw[5],
            digitalRead(CTRL_BTN_MODE), digitalRead(CTRL_BTN_BLINK),
            digitalRead(CTRL_BTN_S2),   digitalRead(CTRL_BTN_S3));
        Serial.println(buf);
    }
#endif
}

// =============================================================================
// Private
// =============================================================================

void ControllerMode::readAndWriteServos() {
    // Head turn
    {
        int   raw    = readPot(CTRL_POT_HEAD_TURN, 0);
        float target = potToDeg(raw, CTRL_POT_HEAD_TURN_MIN, CTRL_POT_HEAD_TURN_MAX,
                                     IDLE_SERVO_RANGE[IDX_HEAD_TURN][0],
                                     IDLE_SERVO_RANGE[IDX_HEAD_TURN][2]);
        int deg = (int)updateSpring(0, target, IDLE_SERVO_RANGE[IDX_HEAD_TURN][0],
                                               IDLE_SERVO_RANGE[IDX_HEAD_TURN][2]);
        _sc.writeDegrees(IDLE_SERVO_CHANNELS[IDX_HEAD_TURN], deg,
                         IDLE_SERVO_RANGE[IDX_HEAD_TURN][0],
                         IDLE_SERVO_RANGE[IDX_HEAD_TURN][2],
                         IDLE_SERVO_REVERSED[IDX_HEAD_TURN]);
    }

    // Head tilt — one pot drives both tilt servos (reversal handles mirroring)
    {
        int   raw    = readPot(CTRL_POT_HEAD_TILT, 1);
        float target = potToDeg(raw, CTRL_POT_HEAD_TILT_MIN, CTRL_POT_HEAD_TILT_MAX,
                                     IDLE_SERVO_RANGE[IDX_HEAD_TILT_L][0],
                                     IDLE_SERVO_RANGE[IDX_HEAD_TILT_L][2]);
        int deg = (int)updateSpring(1, target, IDLE_SERVO_RANGE[IDX_HEAD_TILT_L][0],
                                               IDLE_SERVO_RANGE[IDX_HEAD_TILT_L][2]);
        _sc.writeDegrees(IDLE_SERVO_CHANNELS[IDX_HEAD_TILT_L], deg,
                         IDLE_SERVO_RANGE[IDX_HEAD_TILT_L][0],
                         IDLE_SERVO_RANGE[IDX_HEAD_TILT_L][2],
                         IDLE_SERVO_REVERSED[IDX_HEAD_TILT_L]);
        _sc.writeDegrees(IDLE_SERVO_CHANNELS[IDX_HEAD_TILT_R], deg,
                         IDLE_SERVO_RANGE[IDX_HEAD_TILT_R][0],
                         IDLE_SERVO_RANGE[IDX_HEAD_TILT_R][2],
                         IDLE_SERVO_REVERSED[IDX_HEAD_TILT_R]);
    }

    // Holoprojector
    {
        int   raw    = readPot(CTRL_POT_HOLO, 2);
        float target = potToDeg(raw, CTRL_POT_HOLO_MIN, CTRL_POT_HOLO_MAX,
                                     IDLE_SERVO_RANGE[IDX_HOLO][0],
                                     IDLE_SERVO_RANGE[IDX_HOLO][2]);
        int deg = (int)updateSpring(2, target, IDLE_SERVO_RANGE[IDX_HOLO][0],
                                               IDLE_SERVO_RANGE[IDX_HOLO][2]);
        _sc.writeDegrees(IDLE_SERVO_CHANNELS[IDX_HOLO], deg,
                         IDLE_SERVO_RANGE[IDX_HOLO][0],
                         IDLE_SERVO_RANGE[IDX_HOLO][2],
                         IDLE_SERVO_REVERSED[IDX_HOLO]);
    }

    // Ear left
    {
        int   raw    = readPot(CTRL_POT_EAR_L, 3);
        float target = potToDeg(raw, CTRL_POT_EAR_L_MIN, CTRL_POT_EAR_L_MAX,
                                     IDLE_SERVO_RANGE[IDX_EAR_L][0],
                                     IDLE_SERVO_RANGE[IDX_EAR_L][2]);
        int deg = (int)updateSpring(3, target, IDLE_SERVO_RANGE[IDX_EAR_L][0],
                                               IDLE_SERVO_RANGE[IDX_EAR_L][2]);
        _sc.writeDegrees(IDLE_SERVO_CHANNELS[IDX_EAR_L], deg,
                         IDLE_SERVO_RANGE[IDX_EAR_L][0],
                         IDLE_SERVO_RANGE[IDX_EAR_L][2],
                         IDLE_SERVO_REVERSED[IDX_EAR_L]);
    }

    // Ear right — follows left ear pot when linked, own pot when independent
    {
        float target;
        if (_earsLinked) {
            target = _pos[3]; // track left ear's spring position directly
        } else {
            int raw = readPot(CTRL_POT_EAR_R, 4);
            target  = potToDeg(raw, CTRL_POT_EAR_R_MIN, CTRL_POT_EAR_R_MAX,
                                    IDLE_SERVO_RANGE[IDX_EAR_R][0],
                                    IDLE_SERVO_RANGE[IDX_EAR_R][2]);
        }
        int deg = (int)updateSpring(4, target, IDLE_SERVO_RANGE[IDX_EAR_R][0],
                                               IDLE_SERVO_RANGE[IDX_EAR_R][2]);
        _sc.writeDegrees(IDLE_SERVO_CHANNELS[IDX_EAR_R], deg,
                         IDLE_SERVO_RANGE[IDX_EAR_R][0],
                         IDLE_SERVO_RANGE[IDX_EAR_R][2],
                         IDLE_SERVO_REVERSED[IDX_EAR_R]);
    }

    // CTRL_POT_SPARE (A5) — reserved, no-op
}

int ControllerMode::readPot(uint8_t pin, int idx) {
    int raw = analogRead(pin);
    _smoothed[idx] = CTRL_POT_ALPHA * raw + (1.0f - CTRL_POT_ALPHA) * _smoothed[idx];
    int s = (int)_smoothed[idx];
    if (abs(s - _lastRaw[idx]) > CTRL_POT_DEADBAND) {
        _lastRaw[idx] = s;
    }
    return _lastRaw[idx];
}

float ControllerMode::potToDeg(int raw, int potMin, int potMax, int minDeg, int maxDeg) {
    raw = constrain(raw, potMin, potMax);
    return map(raw, potMin, potMax, minDeg, maxDeg);
}

float ControllerMode::updateSpring(int idx, float target, int minDeg, int maxDeg) {
    _vel[idx] += (target - _pos[idx]) * CTRL_SPRING - _vel[idx] * CTRL_DAMPING;
    _pos[idx] += _vel[idx];
    _pos[idx]  = constrain(_pos[idx], (float)minDeg, (float)maxDeg);
    return _pos[idx];
}

void ControllerMode::updateEye() {
    int   raw    = readPot(CTRL_POT_SPARE, 5);
    float target = potToDeg(raw, CTRL_POT_SPARE_MIN, CTRL_POT_SPARE_MAX,
                                 IDLE_SERVO_RANGE[IDX_EYE_BLINK][0],
                                 IDLE_SERVO_RANGE[IDX_EYE_BLINK][2]);
    int deg = (int)updateSpring(5, target, IDLE_SERVO_RANGE[IDX_EYE_BLINK][0],
                                           IDLE_SERVO_RANGE[IDX_EYE_BLINK][2]);
    _sc.writeDegrees(IDLE_SERVO_CHANNELS[IDX_EYE_BLINK], deg,
                     IDLE_SERVO_RANGE[IDX_EYE_BLINK][0],
                     IDLE_SERVO_RANGE[IDX_EYE_BLINK][2],
                     IDLE_SERVO_REVERSED[IDX_EYE_BLINK]);
}
