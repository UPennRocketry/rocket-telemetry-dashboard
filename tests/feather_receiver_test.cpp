#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>

constexpr int HEX = 16;
void delay(unsigned long) {}

struct FakeSerial {
  std::string output;
  void begin(int) {}
  explicit operator bool() const { return true; }
  void print(const char *text) { output += text; }
  void print(char value) { output += value; }
  void print(int value) { output += std::to_string(value); }
  void print(uint8_t value, int) {
    char text[3];
    snprintf(text, sizeof(text), "%X", value);
    output += text;
  }
  void println(const char *text) { output += text; output += '\n'; }
  void println() { output += '\n'; }
  void write(const uint8_t *bytes, size_t length) {
    output.append(reinterpret_cast<const char *>(bytes), length);
  }
} Serial;

#include "../feather.ino"

FakeLoRa LoRa;

int main() {
  setup();
  Serial.output.clear();
  LoRa.received = "IMU,0,1100,16384,0,-16384,131,0,-131";
  loop();
  assert(Serial.output.find(LoRa.received) != std::string::npos);
  assert(Serial.output.find("Binary payload") == std::string::npos);

  Serial.output.clear();
  LoRa.received = "SIM_FLIGHT,0,-12000,0,0,0,100";
  loop();
  assert(Serial.output.find(LoRa.received) != std::string::npos);
  assert(Serial.output.find("Binary payload") == std::string::npos);

  Serial.output.clear();
  LoRa.received = std::string("\x00\x01\x7f\x80\xff", 5);
  loop();
  assert(Serial.output.find("Binary payload (hex): 00 01 7F 80 FF\n") !=
         std::string::npos);
  assert(Serial.output.find('\0') == std::string::npos);

  Serial.output.clear();
  LoRa.received = "POS,39.952583,-75.165222,123.45";
  loop();
  assert(Serial.output.find("Position (latitude degrees") != std::string::npos);
}
