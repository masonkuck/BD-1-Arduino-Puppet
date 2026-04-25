#pragma once

#include "ServoController.h"
#include "config.h"

//  =============================================================================
//  Keyframe & Sequence types
//  positions[i] = -1 means don't move this servo this frame
//  =============================================================================
struct Keyframe {
    int positions[NUM_IDLE_SERVOS];   // degrees, or -1 to skip
    uint16_t durationMs;              // time to reach this position
};

struct Sequence {
    const char*     name;
    const Keyframe* frames;
    uint8_t         frameCount;
    uint8_t         weight;     // higher = more likely picked during calm
    bool            burstOnly;  // true = only used during burst mode
};

//  =============================================================================
//  IdleMode
//  Plays scripted sequences with eased interpolation.
//  Blink runs on an independent timer (ready to wire in when eyes are built).
//  =============================================================================
class IdleMode {
public:
    IdleMode(ServoController& sc);

    void begin();    // Center servos, pick first sequence
    void update();   // Call every loop() — non-blocking

private:
    enum class State { CALM, BURST, SETTLE };

    ServoController& _sc;

    //  --- Sequence player ---
    State       _state;
    int         _currentSeq;       // Index into sequence table
    int         _currentFrame;     // Index into current sequence's frames
    float       _currentPos[NUM_IDLE_SERVOS];   // Interpolated positions
    float       _startPos[NUM_IDLE_SERVOS];     // Position at frame start
    uint32_t    _frameStartMs;     // millis() when current frame began
    int         _burstRemaining;   // Sequences left in a burst

    //  --- Blink (independent timer, not part of keyframe system) ---
    uint32_t    _nextBlinkMs;
    bool        _blinkActive;
    uint8_t     _blinkPhase;    // 0=closing  1=hold  2=opening
    uint32_t    _blinkPhaseMs;

    //  --- Methods ---
    void        pickNextSequence();
    void        startFrame(int frameIdx);
    void        updateInterpolation();
    void        writePositions();
    void        updateBlink();

    //  Ease in-out (cubic) t in [0,1] → eased t
    float       ease(float t);

    //  Pick a random sequence index from the calm pool (weighted)
    int         pickCalm();
    int         pickBurst();
};