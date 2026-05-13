//PINOUT
// MPU6050 VCC -> ESP32 3V3
// MPU6050 GND -> ESP32 GND
// MPU6050 SDA -> ESP32 GPIO21
// MPU6050 SCL -> ESP32 GPIO22
// MPU6050 AD0 -> GND (I2C address 0x68)
// MPU6050 INT -> optional, leave unconnected for this test
// MPU6050 XDA -> leave unconnected
// MPU6050 XCL -> leave unconnected


#include <Wire.h>

  const uint8_t MPU_ADDR = 0x68; // AD0->GND: 0x68, AD0->3V3: 0x69
  const int SDA_PIN = 19;
  const int SCL_PIN = 18;

  int16_t axRaw, ayRaw, azRaw, gxRaw, gyRaw, gzRaw, tempRaw;

  bool writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.write(val);
    return (Wire.endTransmission() == 0);
  }

  bool readBytes(uint8_t startReg, uint8_t *buf, uint8_t len) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(startReg);
    if (Wire.endTransmission(false) != 0) return false; // repeated start
    uint8_t n = Wire.requestFrom((int)MPU_ADDR, (int)len, (int)true);
    if (n != len) return false;
    for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
    return true;
  }

  void setup() {
    Serial.begin(115200);
    delay(1000);

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000);

    Serial.println("MPU-6050 scaled output test");

    // Wake up MPU-6050
    if (!writeReg(0x6B, 0x00)) {
      Serial.println("ERROR: MPU not responding. Check wiring/address.");
      while (1) delay(1000);
    }

    // Full-scale ranges:
    // ACCEL_CONFIG (0x1C): 0x00 => +/-2g
    // GYRO_CONFIG  (0x1B): 0x00 => +/-250 deg/s
    writeReg(0x1C, 0x00);
    writeReg(0x1B, 0x00);

    uint8_t who = 0;
    if (readBytes(0x75, &who, 1)) {
      Serial.print("WHO_AM_I: 0x");
      Serial.println(who, HEX); // expected 0x68
    }
  }

  void loop() {
    uint8_t d[14];
    if (!readBytes(0x3B, d, 14)) {
      Serial.println("Read error (I2C)");
      delay(500);
      return;
    }

    axRaw = (int16_t)(d[0] << 8 | d[1]);
    ayRaw = (int16_t)(d[2] << 8 | d[3]);
    azRaw = (int16_t)(d[4] << 8 | d[5]);
    tempRaw = (int16_t)(d[6] << 8 | d[7]);
    gxRaw = (int16_t)(d[8] << 8 | d[9]);
    gyRaw = (int16_t)(d[10] << 8 | d[11]);
    gzRaw = (int16_t)(d[12] << 8 | d[13]);

    // Sensitivity for selected ranges:
    // +/-2g     => 16384 LSB/g
    // +/-250dps => 131 LSB/(deg/s)
    float ax_g = axRaw / 16384.0f;
    float ay_g = ayRaw / 16384.0f;
    float az_g = azRaw / 16384.0f;

    float gx_dps = gxRaw / 131.0f;
    float gy_dps = gyRaw / 131.0f;
    float gz_dps = gzRaw / 131.0f;

    float tempC = (tempRaw / 340.0f) + 36.53f;

    Serial.print("ACC[g] ");
    Serial.print(ax_g, 3); Serial.print(", ");
    Serial.print(ay_g, 3); Serial.print(", ");
    Serial.print(az_g, 3);

    Serial.print(" | GYRO[dps] ");
    Serial.print(gx_dps, 2); Serial.print(", ");
    Serial.print(gy_dps, 2); Serial.print(", ");
    Serial.print(gz_dps, 2);

    Serial.print(" | Temp[C] ");
    Serial.println(tempC, 2);

    delay(200);
  }
