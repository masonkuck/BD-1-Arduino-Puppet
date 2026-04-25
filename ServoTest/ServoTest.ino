// =============================================================================
// BD-1 Servo Test
// One pot sweeps the selected servo through its full range.
// Two buttons step forward/backward through all servo channels.
// Serial prints the current servo, degree value, and raw pot reading.
// =============================================================================

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// PCA9685 settings — must match config.h
#define PWM_FREQ            50
#define OSCILLATOR_FREQ     25000000
#define SERVO_MIN_US        500
#define SERVO_MAX_US        2500

// Control pot (A5 — holoprojector knob, not on joystick)
#define POT_CTRL            A5

// Step buttons (active LOW, INPUT_PULLUP)
#define BTN_PREV            7   // D7 — step to previous servo
#define BTN_NEXT            9   // D9 — step to next servo

#define DEBOUNCE_MS         50
#define PRINT_INTERVAL_MS   100

// =============================================================================
// Servo table
// Add or remove rows to change which servos are tested and in what order.
// channel  — PCA9685 output number (matches BD1 config.h CH_ defines)
// minDeg   — position when pot is at its low end (0° = 500µs)
// maxDeg   — position when pot is at its high end (180° = 2500µs)
// 90° always equals physical center (1500µs) regardless of min/max.
// NUM_SERVOS is computed automatically — no need to update it manually.
// =============================================================================
struct ServoEntry {
    const char* name;
    uint8_t     channel;
    int         minDeg;
    int         maxDeg;
};

const ServoEntry SERVOS[] = {
    { "Head Turn",      1,  0, 115 },  // Ch 1  — 40° each side of center
    { "Tilt Left",      2,  50, 110 },  // Ch 2  — left tilt servo
    { "Tilt Right",     3,  50, 110 },  // Ch 3  — right tilt servo (physically reversed)
    { "Eye Blink",      5,   0, 130 },  // Ch 5  — 0°=closed, 130°=open
    { "Holoprojector",  6,  75, 105 },  // Ch 6  — small pivot sweep
    { "Ear Left",      14,   0, 120 },  // Ch 14 — 0=closed, 120=open
    { "Ear Right",     15,   0, 120 },  // Ch 15 — 0=closed, 120=open
};
const int NUM_SERVOS = (int)(sizeof(SERVOS) / sizeof(SERVOS[0]));

// =============================================================================
// Globals
// =============================================================================
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);

int      currentIdx       = 0;
uint32_t _lastPrintMs     = 0;

// Debounce state for each button
bool     _prevRaw         = HIGH, _prevDebounced = HIGH;
bool     _nextRaw         = HIGH, _nextDebounced = HIGH;
uint32_t _prevDebounceMs  = 0,    _nextDebounceMs = 0;

// =============================================================================
// Helpers
// =============================================================================
void writeServoDeg(uint8_t channel, int deg) {
    int      us  = map(deg, 0, 180, SERVO_MIN_US, SERVO_MAX_US);
    uint16_t pwmVal = (uint16_t)((us / (1000000.0f / PWM_FREQ)) * 4096);
    pwm.setPWM(channel, 0, pwmVal);
}

// Returns true on the falling edge (button just pressed)
bool debounce(uint8_t pin, bool& raw, bool& debounced, uint32_t& lastMs) {
    bool reading = digitalRead(pin);
    if (reading != raw) { raw = reading; lastMs = millis(); }
    if ((millis() - lastMs) > DEBOUNCE_MS && raw != debounced) {
        debounced = raw;
        if (debounced == LOW) return true;
    }
    return false;
}

void printSelection() {
    const ServoEntry& s = SERVOS[currentIdx];
    Serial.print("[");
    Serial.print(currentIdx + 1);
    Serial.print("/");
    Serial.print(NUM_SERVOS);
    Serial.print("]  Ch ");
    Serial.print(s.channel);
    Serial.print("  ");
    Serial.print(s.name);
    Serial.print("  (");
    Serial.print(s.minDeg);
    Serial.print("° – ");
    Serial.print(s.maxDeg);
    Serial.println("°)");
}

// =============================================================================
// Setup / Loop
// =============================================================================
void setup() {
    Serial.begin(9600);

    pinMode(BTN_PREV, INPUT_PULLUP);
    pinMode(BTN_NEXT, INPUT_PULLUP);

    pwm.begin();
    pwm.setOscillatorFrequency(OSCILLATOR_FREQ);
    pwm.setPWMFreq(PWM_FREQ);
    delay(100);

    Serial.println("=== BD-1 Servo Test ===");
    Serial.println("D7=Prev  D9=Next  A5=Position");
    Serial.println("----------------------------------------");
    printSelection();
}

void loop() {
    // Step backward
    if (debounce(BTN_PREV, _prevRaw, _prevDebounced, _prevDebounceMs)) {
        currentIdx = (currentIdx - 1 + NUM_SERVOS) % NUM_SERVOS;
        printSelection();
    }

    // Step forward
    if (debounce(BTN_NEXT, _nextRaw, _nextDebounced, _nextDebounceMs)) {
        currentIdx = (currentIdx + 1) % NUM_SERVOS;
        printSelection();
    }

    // Read pot and drive selected servo
    const ServoEntry& s = SERVOS[currentIdx];
    int raw = analogRead(POT_CTRL);
    int deg = map(raw, 0, 1023, s.minDeg, s.maxDeg);
    writeServoDeg(s.channel, deg);

    // Periodic serial output
    if (millis() - _lastPrintMs >= PRINT_INTERVAL_MS) {
        _lastPrintMs = millis();
        char buf[40];
        snprintf(buf, sizeof(buf), "  deg=%-4d  raw=%-4d", deg, raw);
        Serial.println(buf);
    }
}
