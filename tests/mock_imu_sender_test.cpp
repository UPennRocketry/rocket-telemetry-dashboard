#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>

unsigned long clockMs = 0;
unsigned long millis() { return clockMs; }
void delay(unsigned long ms) { clockMs += ms; }

struct FakeSerial {
  std::string output;
  void begin(int) {}
  void print(const char *text) { output += text; }
  void println(const char *text) { output += text; output += '\n'; }
} Serial;

#include "../mock_imu_sender/mock_imu_sender.ino"

FakeLoRa LoRa;

int main(int argc, char **argv) {
  if (argc == 2 && std::string(argv[1]) == "--samples") {
    for (size_t i = 0; i < FLIGHT_SAMPLE_COUNT; ++i) {
      const FlightSample sample = flightSample(i);
      printf("%ld,%ld,%ld,%ld,%ld\n", static_cast<long>(sample.timeMs),
             sample.altitudeFt, sample.downrangeFt,
             sample.velocityTenthsMph, sample.accelerationHundredthsG);
    }
    return 0;
  }
  setup();
  loop();
  assert(LoRa.packets.size() == 1);
  assert(LoRa.packets[0] == "SIM_FLIGHT,0,-12000,0,0,0,100");
  clockMs += SEND_INTERVAL_MS - 1;
  loop();
  assert(LoRa.packets.size() == 1);
  ++clockMs;
  loop();
  assert(LoRa.packets.size() == 2);
  for (size_t i = 2; i < FLIGHT_SAMPLE_COUNT; ++i) {
    clockMs += SEND_INTERVAL_MS;
    loop();
  }
  assert(LoRa.packets.size() == FLIGHT_SAMPLE_COUNT);
  assert(LoRa.packets[12] == "SIM_FLIGHT,12,0,0,0,0,100");
  assert(LoRa.packets[20] == "SIM_FLIGHT,20,8000,2651,239,1464,46");
  assert(LoRa.packets[140] == "SIM_FLIGHT,140,128000,0,2220,0,100");
  for (const std::string &packet : LoRa.packets) {
    assert(packet.size() <= 255);
  }
  clockMs += SEND_INTERVAL_MS;
  loop();
  assert(LoRa.packets.back() == "SIM_FLIGHT,145,-12000,0,0,0,100");
}
