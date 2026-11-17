#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>
#include "imu_packet.h"

// Feather M0 with built-in RFM9x radio; must match feather.ino.
constexpr int RFM95_CS = 8;
constexpr int RFM95_RST = 4;
constexpr int RFM95_INT = 3;
constexpr long RADIO_FREQUENCY_HZ = 433000000;
constexpr long RADIO_BANDWIDTH_HZ = 125000;
constexpr int RADIO_SPREADING_FACTOR = 7;
constexpr int RADIO_CODING_RATE = 5;
constexpr int RADIO_SYNC_WORD = 0x12;
constexpr unsigned long SEND_INTERVAL_MS = 1000;

uint8_t imuAddress = 0;
uint32_t sequence = 0;
unsigned long lastAttempt = 0;

bool readRegisters(uint8_t address, uint8_t reg, uint8_t *data, uint8_t count) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }
  if (Wire.requestFrom(address, count) != count) {
    while (Wire.available()) {
      Wire.read();
    }
    return false;
  }
  for (uint8_t i = 0; i < count; ++i) {
    data[i] = Wire.read();
  }
  return true;
}

bool writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool initializeImu() {
  for (uint8_t address = 0x68; address <= 0x69; ++address) {
    Wire.beginTransmission(address);
    const uint8_t status = Wire.endTransmission();
    Serial.print("I2C address 0x");
    Serial.print(address, HEX);
    if (status != 0) {
      Serial.print(": no ACK or bus error, Wire status ");
      Serial.println(status);
      continue;
    }

    Serial.print(": ACK; ");
    uint8_t identity;
    if (!readRegisters(address, 0x75, &identity, 1)) {
      Serial.println("ERROR: WHO_AM_I register read failed.");
      continue;
    }
    Serial.print("WHO_AM_I=0x");
    Serial.println(identity, HEX);
    if ((identity & 0x7e) != 0x68) {
      Serial.println("ERROR: Unexpected MPU6050 identity.");
      continue;
    }
    if (!writeRegister(address, 0x6b, 0x00) ||
        !writeRegister(address, 0x1b, 0x00) ||
        !writeRegister(address, 0x1c, 0x00)) {
      Serial.println("ERROR: MPU6050 configuration failed.");
      return false;
    }
    delay(100);
    imuAddress = address;
    Serial.print("MPU6050 initialized at I2C address 0x");
    Serial.println(imuAddress, HEX);
    return true;
  }
  Serial.println("ERROR: Could not initialize an MPU6050 at 0x68 or 0x69; retrying.");
  return false;
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  LoRa.setPins(RFM95_CS, RFM95_RST, RFM95_INT);
  while (!LoRa.begin(RADIO_FREQUENCY_HZ)) {
    Serial.println("ERROR: Radio initialization failed; retrying in 5 seconds.");
    delay(5000);
  }
  LoRa.setSignalBandwidth(RADIO_BANDWIDTH_HZ);
  LoRa.setSpreadingFactor(RADIO_SPREADING_FACTOR);
  LoRa.setCodingRate4(RADIO_CODING_RATE);
  LoRa.setSyncWord(RADIO_SYNC_WORD);
  Serial.println("IMU sender started on 433 MHz.");
  lastAttempt = millis() - SEND_INTERVAL_MS;
}

void loop() {
  const unsigned long now = millis();
  if (now - lastAttempt < SEND_INTERVAL_MS) {
    return;
  }
  lastAttempt = now;

  if (imuAddress == 0 && !initializeImu()) {
    return;
  }

  uint8_t data[14];
  if (!readRegisters(imuAddress, 0x3b, data, sizeof(data))) {
    Serial.println("ERROR: MPU6050 read failed; will retry initialization.");
    imuAddress = 0;
    return;
  }

  char packet[96];
  const int length = formatImuPacket(packet, sizeof(packet), sequence, millis(), data);
  if (length < 0 || static_cast<size_t>(length) >= sizeof(packet)) {
    Serial.println("ERROR: IMU packet formatting failed.");
    return;
  }
  if (!LoRa.beginPacket()) {
    Serial.println("ERROR: Radio busy; IMU packet not sent.");
    return;
  }
  const size_t written = LoRa.write(reinterpret_cast<const uint8_t *>(packet), length);
  const int sent = LoRa.endPacket();
  if (written != static_cast<size_t>(length) || sent != 1) {
    Serial.println("ERROR: IMU packet transmission failed.");
    return;
  }
  Serial.print("Sent: ");
  Serial.println(packet);
  ++sequence;
}
