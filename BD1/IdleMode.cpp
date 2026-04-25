#include "IdleMode.h"

// =============================================================================
// Sequence Definitions
// IDX order: HEAD_TURN, HEAD_TILT_L, HEAD_TILT_R, HOLO, EAR_L, EAR_R, EYE_BLINK
// Ranges from config.h:
//   HEAD_TURN:  50-130  center 90
//   TILT L/R:   50-110  center 90
//   HOLO:       75-105  center 90
//   EARS:       75-105  center 90
//   EYE_BLINK:  independently controlled — always -1 in keyframes
// -1 = don't move this servo in this keyframe
// =============================================================================

// --- IDLE_DRIFT: slow relaxed wander, BD-1 at rest ---
static const Keyframe SEQ_DRIFT_FRAMES[] = {
//  { TURN, TILT_L, TILT_R, HOLO,  EAR_L, EAR_R, BLINK }, ms
    { { 105,    83,     83,   96,    90,    90,   -1 }, 3500 },
    { {  78,    95,     95,   84,    90,    90,   -1 }, 4000 },
    { {  98,    84,     84,   95,    90,    90,   -1 }, 3500 },
    { {  90,    90,     90,   90,    90,    90,   -1 }, 2500 },
};

// --- LOOK_LEFT: slow turn left, ear follows ---
static const Keyframe SEQ_LOOK_LEFT_FRAMES[] = {
    { {  60,    90,     90,   85,    95,    95,   -1 }, 1200 },
    { {  60,    90,     90,   85,    95,    95,   -1 }, 800  }, // hold
    { {  90,    90,     90,   90,    90,    90,   -1 }, 1000 },
};

// --- LOOK_RIGHT: slow turn right, ear follows ---
static const Keyframe SEQ_LOOK_RIGHT_FRAMES[] = {
    { { 120,    90,     90,   95,    85,    85,   -1 }, 1200 },
    { { 120,    90,     90,   95,    85,    85,   -1 }, 800  }, // hold
    { {  90,    90,     90,   90,    90,    90,   -1 }, 1000 },
};

// --- CURIOUS_TILT: head tilts, holo drifts curiously ---
static const Keyframe SEQ_CURIOUS_TILT_FRAMES[] = {
    { {  90,    50,     50,   85,    90,    90,   -1 }, 1000 },
    { {  88,    50,     50,   100,   90,    90,   -1 }, 800  }, // holo looks up
    { {  90,    90,     90,   90,    90,    90,   -1 }, 1200 },
};

// --- SCAN: slow sweep across range, holo tracks ---
static const Keyframe SEQ_SCAN_FRAMES[] = {
    { {  55,    90,     90,   75,    90,    90,   -1 }, 1800 },
    { { 125,    90,     90,   105,   90,    90,   -1 }, 3000 },
    { {  90,    90,     90,   90,    90,    90,   -1 }, 1200 },
};

// --- EAR_TWITCH: quick ear flick, subtle ---
static const Keyframe SEQ_EAR_TWITCH_FRAMES[] = {
    { {  -1,    -1,     -1,   -1,    100,   100,  -1 }, 150  },
    { {  -1,    -1,     -1,   -1,    80,    80,   -1 }, 150  },
    { {  -1,    -1,     -1,   -1,    90,    90,   -1 }, 300  },
};

// --- SETTLE: slow drift back to neutral, used after burst ---
static const Keyframe SEQ_SETTLE_FRAMES[] = {
    { {  90,    90,     90,   90,    90,    90,   -1 }, 2000 },
    { {  90,    90,     90,   90,    90,    90,   -1 }, 1000 }, // hold neutral
};

// --- STARTLE: quick multi-axis snap, burst only ---
static const Keyframe SEQ_STARTLE_FRAMES[] = {
    { {  65,    76,     76,   105,   100,   100,  -1 }, 500  }, // snap
    { {  65,    76,     76,   105,   100,   100,  -1 }, 800  }, // hold
    { { 115,    98,     98,   80,    80,    80,   -1 }, 500  }, // snap other way
    { {  90,    90,     90,   90,    90,    90,   -1 }, 800  }, // settle
};

// --- EXCITED_SCAN: fast energetic look-around, burst only ---
static const Keyframe SEQ_EXCITED_SCAN_FRAMES[] = {
    { {  60,    78,     78,   75,    100,   100,  -1 }, 400  },
    { { 120,    98,     98,   105,   80,    80,   -1 }, 500  },
    { {  80,    80,     80,   85,    90,    90,   -1 }, 350  },
    { {  90,    90,     90,   90,    90,    90,   -1 }, 600  },
};

// =============================================================================
// Sequence Table
//
// weight: controls how often a calm sequence is randomly picked.
//   Probability = weight / sum of all non-zero calm weights.
//   Example: DRIFT=40, LOOK_LEFT=20 → DRIFT fires 67%, LOOK_LEFT 33%.
//   Set weight=0 to disable a calm sequence without deleting it.
//   burstOnly sequences (weight=0, burstOnly=true) are never calm picks —
//   they only play during a burst. SETTLE is always weight=0 and is
//   triggered directly by the state machine after a burst.
//
// TUNING: restore normal weights once behaviour is dialled in:
//   DRIFT=40, LOOK_LEFT=20, LOOK_RIGHT=20, CURIOUS_TILT=15,
//   SCAN=10, EAR_TWITCH=30
// =============================================================================
static const Sequence SEQUENCES[] = {
    { "DRIFT",        SEQ_DRIFT_FRAMES,        4,  1, false },
    { "LOOK_LEFT",    SEQ_LOOK_LEFT_FRAMES,    3,  0, false },
    { "LOOK_RIGHT",   SEQ_LOOK_RIGHT_FRAMES,   3,  0, false },
    { "CURIOUS_TILT", SEQ_CURIOUS_TILT_FRAMES, 3,  1, false },
    { "SCAN",         SEQ_SCAN_FRAMES,         3,  0, false },
    { "EAR_TWITCH",   SEQ_EAR_TWITCH_FRAMES,   3,  0, false },
    { "SETTLE",       SEQ_SETTLE_FRAMES,        2,  0, false }, // triggered by state machine, not randomly
    { "STARTLE",      SEQ_STARTLE_FRAMES,       4,  0, true  },
    { "EXCITED_SCAN", SEQ_EXCITED_SCAN_FRAMES,  4,  0, true  },
};

static const int SEQ_COUNT           = sizeof(SEQUENCES) / sizeof(SEQUENCES[0]);
static const int SEQ_IDX_SETTLE      = 6; // Must match SEQUENCES table above
static const int SEQ_IDX_BURST_START = 7; // First burst-only sequence index

// =============================================================================
// IdleMode implementation
// =============================================================================

IdleMode::IdleMode(ServoController& sc) : _sc(sc) {}

void IdleMode::begin() {
    _state          = State::CALM;
    _burstRemaining = 0;
    _currentSeq     = pickCalm();
    _currentFrame   = 0;
    _frameStartMs   = millis();
    _nextBlinkMs    = millis() + random(IDLE_BLINK_MIN_MS, IDLE_BLINK_MAX_MS);
    _blinkActive    = false;

    // Initialise current positions to center
    for (int i = 0; i < NUM_IDLE_SERVOS; i++) {
        _currentPos[i] = IDLE_SERVO_RANGE[i][1];
        _startPos[i]   = _currentPos[i];
    }

    startFrame(0);
    Serial.println("[IdleMode] Started.");
}

void IdleMode::update() {
    updateInterpolation();
    writePositions();
    updateBlink();
}

// =============================================================================
// Private
// =============================================================================

void IdleMode::startFrame(int frameIdx) {
    _currentFrame = frameIdx;
    _frameStartMs = millis();

    const Keyframe& kf = SEQUENCES[_currentSeq].frames[frameIdx];

    for (int i = 0; i < NUM_IDLE_SERVOS; i++) {
        _startPos[i] = _currentPos[i];
        // If -1, target stays at current position (servo doesn't move)
        if (kf.positions[i] == -1) {
            // leave _startPos as-is so interpolation stays put
        }
    }
}

void IdleMode::updateInterpolation() {
    const Sequence& seq = SEQUENCES[_currentSeq];
    const Keyframe& kf  = seq.frames[_currentFrame];

    uint32_t elapsed = millis() - _frameStartMs;
    float t  = constrain((float)elapsed / (float)kf.durationMs, 0.0f, 1.0f);
    float et = ease(t);

    for (int i = 0; i < NUM_IDLE_SERVOS; i++) {
        if (kf.positions[i] == -1) continue; // skip — servo holds
        float target = (float)kf.positions[i];
        _currentPos[i] = _startPos[i] + (target - _startPos[i]) * et;
    }

    // Frame complete?
    if (t >= 1.0f) {
        // Update startPos to the frame's target for next frame
        for (int i = 0; i < NUM_IDLE_SERVOS; i++) {
            if (kf.positions[i] != -1) {
                _startPos[i]   = (float)kf.positions[i];
                _currentPos[i] = _startPos[i];
            }
        }

        int nextFrame = _currentFrame + 1;
        if (nextFrame < seq.frameCount) {
            startFrame(nextFrame);
        } else {
            pickNextSequence();
        }
    }
}

void IdleMode::pickNextSequence() {
    switch (_state) {
        case State::CALM:
            if (random(100) < IDLE_BURST_CHANCE) {
                _state          = State::BURST;
                _burstRemaining = IDLE_BURST_SEQ_COUNT;
                _currentSeq     = pickBurst();
            } else {
                _currentSeq = pickCalm();
            }
            break;

        case State::BURST:
            _burstRemaining--;
            if (_burstRemaining > 0) {
                _currentSeq = pickBurst();
            } else {
                _state      = State::SETTLE;
                _currentSeq = SEQ_IDX_SETTLE;
            }
            break;

        case State::SETTLE:
            _state      = State::CALM;
            _currentSeq = pickCalm();
            break;
    }

    _currentFrame = 0;
    _frameStartMs = millis();
    startFrame(0);

    Serial.print("[IdleMode] -> ");
    Serial.println(SEQUENCES[_currentSeq].name);
}

void IdleMode::writePositions() {
    for (int i = 0; i < NUM_IDLE_SERVOS; i++) {
        if (i == IDX_EYE_BLINK) continue; // blink controlled independently in updateBlink()
        int deg = (int)_currentPos[i];
        _sc.writeDegrees(
            IDLE_SERVO_CHANNELS[i],
            deg,
            IDLE_SERVO_RANGE[i][0],
            IDLE_SERVO_RANGE[i][2],
            IDLE_SERVO_REVERSED[i]
        );
    }
}

void IdleMode::updateBlink() {
    uint32_t now = millis();

    if (!_blinkActive) {
        if (now >= _nextBlinkMs) {
            _blinkActive  = true;
            _blinkPhase   = 0;
            _blinkPhaseMs = now;
            _nextBlinkMs  = now + random(IDLE_BLINK_MIN_MS, IDLE_BLINK_MAX_MS);
        }
        return;
    }

    uint32_t elapsed  = now - _blinkPhaseMs;
    int      openDeg  = IDLE_SERVO_RANGE[IDX_EYE_BLINK][1]; // center = open
    int      closedDeg = IDLE_SERVO_RANGE[IDX_EYE_BLINK][0]; // min = closed
    uint8_t  ch       = IDLE_SERVO_CHANNELS[IDX_EYE_BLINK];
    bool     rev      = IDLE_SERVO_REVERSED[IDX_EYE_BLINK];

    switch (_blinkPhase) {
        case 0: { // closing
            float t  = constrain((float)elapsed / BLINK_CLOSE_MS, 0.0f, 1.0f);
            int   deg = (int)(openDeg + (closedDeg - openDeg) * t);
            _sc.writeDegrees(ch, deg, closedDeg, openDeg, rev);
            if (t >= 1.0f) { _blinkPhase = 1; _blinkPhaseMs = now; }
            break;
        }
        case 1: { // hold closed
            if (elapsed >= BLINK_HOLD_MS) { _blinkPhase = 2; _blinkPhaseMs = now; }
            break;
        }
        case 2: { // opening
            float t   = constrain((float)elapsed / BLINK_OPEN_MS, 0.0f, 1.0f);
            int   deg = (int)(closedDeg + (openDeg - closedDeg) * t);
            _sc.writeDegrees(ch, deg, closedDeg, openDeg, rev);
            if (t >= 1.0f) { _blinkActive = false; }
            break;
        }
    }
}

float IdleMode::ease(float t) {
    // Cubic ease in-out
    return t < 0.5f ? 4.0f * t * t * t
                    : 1.0f - pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

int IdleMode::pickCalm() {
    // Weighted random from non-burst sequences
    int totalWeight = 0;
    for (int i = 0; i < SEQ_COUNT; i++) {
        if (!SEQUENCES[i].burstOnly && SEQUENCES[i].weight > 0) {
            totalWeight += SEQUENCES[i].weight;
        }
    }
    int roll = random(totalWeight);
    int acc  = 0;
    for (int i = 0; i < SEQ_COUNT; i++) {
        if (!SEQUENCES[i].burstOnly && SEQUENCES[i].weight > 0) {
            acc += SEQUENCES[i].weight;
            if (roll < acc) return i;
        }
    }
    return 0;
}

int IdleMode::pickBurst() {
    // Random from burst-only sequences
    int count = SEQ_COUNT - SEQ_IDX_BURST_START;
    return SEQ_IDX_BURST_START + random(count);
}
