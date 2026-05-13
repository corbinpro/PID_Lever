#include <Arduino.h>

  // ===== ESP32 pin =====
  const int ADC_PIN = 34; // Sensor OUT -> GPIO34 (ADC1)

  // ===== Timing =====
  const unsigned long STARTUP_BASELINE_MS = 5000;
  const unsigned long LEARN_WINDOW_MS = 10000; // rotate through full travel during this


  // ===== Target angle mapping =====
  const float TARGET_MIN_DEG = -90.0f;
  const float TARGET_MAX_DEG =  90.0f;

  // ===== State =====
  float baselineMv = 0.0f;
  float learnedMinMv = 99999.0f;
  float learnedMaxMv = -99999.0f;
  bool ready = false;

  float readMv(int samples = 16) {
    uint32_t sum = 0;
    for (int i = 0; i < samples; i++) {
      sum += analogReadMilliVolts(ADC_PIN);
      delay(2);
    }
    return (float)sum / samples;
  }

  float mapFloat(float x, float inMin, float inMax, float outMin, float outMax) {
    if (inMax - inMin < 1.0f) return outMin;
    return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
  }

  void setup() {
    Serial.begin(115200);
    delay(400);

    analogReadResolution(12);
    analogSetPinAttenuation(ADC_PIN, ADC_11db); // ~0..3.3V range

    Serial.println("Step 1: Hold desired START position for 5s (this becomes -90 deg)");

    // Baseline calibration (-90 deg reference)
    double acc = 0.0;
    uint32_t n = 0;
    unsigned long t0 = millis();
    while (millis() - t0 < STARTUP_BASELINE_MS) {
      float mv = readMv(8);
      acc += mv;
      n++;
      Serial.print("Cal baseline mV: ");
      Serial.println(mv, 1);
      delay(40);
    }
    baselineMv = (n > 0) ? (float)(acc / n) : readMv();

    // Start learned range with baseline included
    learnedMinMv = baselineMv;
    learnedMaxMv = baselineMv;

    Serial.println();
    Serial.println("Step 2: For next 10s, rotate through full range to learn min/max.");
    Serial.println("If not fully rotated, mapping accuracy will be limited.");

    unsigned long t1 = millis();
    while (millis() - t1 < LEARN_WINDOW_MS) {
      float mv = readMv(8);
      if (mv < learnedMinMv) learnedMinMv = mv;
      if (mv > learnedMaxMv) learnedMaxMv = mv;

      Serial.print("Learning mV: ");
      Serial.print(mv, 1);
      Serial.print(" | min=");
      Serial.print(learnedMinMv, 1);
      Serial.print(" max=");
      Serial.println(learnedMaxMv, 1);
      delay(40);
    }

    // Force baseline to correspond to -90 by making sure it is the low endpoint if needed
    // (common case: clockwise increases voltage; if reversed, mapping below still works with

    if (baselineMv > learnedMinMv) learnedMinMv = baselineMv;

    Serial.println();
    Serial.println("Calibration complete.");
    Serial.print("Baseline (-90 deg) mV: "); Serial.println(baselineMv, 1);
    Serial.print("Learned min mV: ");        Serial.println(learnedMinMv, 1);
    Serial.print("Learned max mV: ");        Serial.println(learnedMaxMv, 1);
    Serial.println("---- Live angle output ----");

    ready = true;
  }

  void loop() {
    if (!ready) return;

    float mv = readMv(12);

    // Map learned voltage range to -90..+90
    float angle = mapFloat(mv, learnedMinMv, learnedMaxMv, TARGET_MIN_DEG, TARGET_MAX_DEG);

    // Clamp output
    if (angle < TARGET_MIN_DEG) angle = TARGET_MIN_DEG;
    if (angle > TARGET_MAX_DEG) angle = TARGET_MAX_DEG;

    Serial.print("V=");
    Serial.print(mv, 1);
    Serial.print(" mV, angle=");
    Serial.print(angle, 2);
    Serial.println(" deg");

    delay(80);
  }
