  const int RPWM_PIN = 33;
  const int LPWM_PIN = 32;

  const int RPWM_CH = 0;
  const int LPWM_CH = 1;

  const int PWM_FREQ = 8000;
  const int PWM_RES  = 10;          // 0..1023
  const uint16_t DUTY_MAX = 1023; //from power supply, max: 525

  void setup() {
    ledcAttachChannel(RPWM_PIN, PWM_FREQ, PWM_RES, RPWM_CH);
    ledcAttachChannel(LPWM_PIN, PWM_FREQ, PWM_RES, LPWM_CH);
    stopMotor();
  }

  void stopMotor() {
    ledcWrite(RPWM_PIN, 0);
    ledcWrite(LPWM_PIN, 0);
  }

  void setForward(uint16_t duty) {
    ledcWrite(LPWM_PIN, 0);
    ledcWrite(RPWM_PIN, duty);
  }

  void setReverse(uint16_t duty) {
    ledcWrite(RPWM_PIN, 0);
    ledcWrite(LPWM_PIN, duty);
  }

  void rampForward(uint16_t maxDuty, uint16_t step, uint16_t stepDelayMs) {
    for (uint16_t d = 0; d <= maxDuty; d += step) {
      setForward(d);
      delay(stepDelayMs);
    }
    setForward(maxDuty);
  }

  void rampReverse(uint16_t maxDuty, uint16_t step, uint16_t stepDelayMs) {
    for (uint16_t d = 0; d <= maxDuty; d += step) {
      setReverse(d);
      delay(stepDelayMs);
    }
    setReverse(maxDuty);
  }

  void loop() {
    //rampForward(DUTY_MAX, 8, 1);   // 0 -> full forward
    //delay(2000);

    setForward(100);
    delay(2000);

    setForward(200);
    delay(2000);

    setForward(400);
    delay(2000);

    setForward(600);
    delay(2000);

    setForward(800);
    delay(2000);

    setForward(1023);
    delay(2000);

    
  }
