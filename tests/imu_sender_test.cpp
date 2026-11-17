#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>

unsigned long clockMs = 0;
unsigned long millis() { return clockMs; }
void delay(unsigned long ms) { clockMs += ms; }
constexpr int HEX = 16;

struct FakeSerial {
  std::string output;
  void begin(int) {}
  void print(const char *text) { output += text; }
  void print(uint8_t value, int) {
    char text[3];
    snprintf(text, sizeof(text), "%X", value);
    output += text;
  }
  void println(uint8_t value) { output += std::to_string(value) + "\n"; }
  void println(const char *text) { output += text; output += '\n'; }
  void println(uint8_t value, int base) { print(value, base); output += '\n'; }
} Serial;

#include "../imu_sender/imu_sender.ino"

FakeWire Wire;
FakeLoRa LoRa;

int main() {
  assert(SEND_INTERVAL_MS == 1000);
  setup();
  loop();
  assert(LoRa.packets.empty());
  assert(Serial.output.find("I2C address 0x68: no ACK or bus error, Wire status 2") != std::string::npos);
  assert(Serial.output.find("I2C address 0x69: no ACK or bus error, Wire status 2") != std::string::npos);
  assert(Serial.output.find("Could not initialize an MPU6050") != std::string::npos);

  Wire.sensorPresent = true;
  Wire.identity = 0x42;
  clockMs += SEND_INTERVAL_MS;
  loop();
  assert(LoRa.packets.empty());
  assert(Serial.output.find("I2C address 0x68: ACK; WHO_AM_I=0x42") != std::string::npos);
  assert(Serial.output.find("Unexpected MPU6050 identity") != std::string::npos);

  Wire.identity = 0x68;
  Wire.sensorAddress = 0x69;
  clockMs += SEND_INTERVAL_MS - 1;
  loop();
  assert(LoRa.packets.empty());
  ++clockMs;
  loop();
  assert(LoRa.packets.size() == 1);
  assert(LoRa.packets[0] == "IMU,0,2100,16384,0,-16384,131,0,-131");
  assert(Serial.output.find("I2C address 0x69: ACK; WHO_AM_I=0x68") != std::string::npos);

  clockMs += SEND_INTERVAL_MS;
  Wire.failRead = true;
  loop();
  assert(LoRa.packets.size() == 1);
  assert(Serial.output.find("MPU6050 read failed") != std::string::npos);

  Wire.failRead = false;
  clockMs += SEND_INTERVAL_MS;
  loop();
  assert(LoRa.packets.size() == 2);
  assert(LoRa.packets[1].find("IMU,1,") == 0);

  imuAddress = 0;
  Wire.failRead = true;
  clockMs += SEND_INTERVAL_MS;
  loop();
  assert(LoRa.packets.size() == 2);
  assert(Serial.output.find("ERROR: WHO_AM_I register read failed.") != std::string::npos);
}
