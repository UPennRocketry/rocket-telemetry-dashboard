#ifndef TEST_WIRE_H
#define TEST_WIRE_H
#include <stdint.h>
#include <vector>

class FakeWire {
 public:
  bool sensorPresent = false;
  bool failRead = false;
  uint8_t sensorAddress = 0x68;
  uint8_t identity = 0x68;
  uint8_t address = 0;
  std::vector<uint8_t> outgoing;
  std::vector<uint8_t> incoming;
  size_t position = 0;

  void begin() {}
  void beginTransmission(uint8_t target) {
    address = target;
    outgoing.clear();
  }
  void write(uint8_t value) { outgoing.push_back(value); }
  int endTransmission(bool = true) {
    return sensorPresent && address == sensorAddress ? 0 : 2;
  }
  uint8_t requestFrom(uint8_t, uint8_t count) {
    incoming.clear();
    position = 0;
    if (failRead) return 0;
    if (outgoing[0] == 0x75) {
      incoming.push_back(identity);
    } else {
      incoming = {0x40, 0, 0, 0, 0xc0, 0, 0, 0, 0, 0x83, 0, 0, 0xff, 0x7d};
    }
    return count == incoming.size() ? count : 0;
  }
  int available() { return static_cast<int>(incoming.size() - position); }
  int read() { return incoming[position++]; }
};

extern FakeWire Wire;
#endif
