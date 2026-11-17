#ifndef TEST_LORA_H
#define TEST_LORA_H
#include <stdint.h>
#include <string>
#include <vector>

class FakeLoRa {
 public:
  std::string current;
  std::vector<std::string> packets;
  std::string received;
  size_t readPosition = 0;
  void setPins(int, int, int) {}
  int begin(long) { return 1; }
  void setSignalBandwidth(long) {}
  void setSpreadingFactor(int) {}
  void setCodingRate4(int) {}
  void setSyncWord(int) {}
  int beginPacket() { current.clear(); return 1; }
  size_t write(const uint8_t *data, size_t length) {
    current.assign(reinterpret_cast<const char *>(data), length);
    return length;
  }
  int endPacket() { packets.push_back(current); return 1; }
  int parsePacket() { readPosition = 0; return static_cast<int>(received.size()); }
  int packetRssi() { return -92; }
  int available() { return static_cast<int>(received.size() - readPosition); }
  int read() { return static_cast<uint8_t>(received[readPosition++]); }
};

extern FakeLoRa LoRa;
#endif
