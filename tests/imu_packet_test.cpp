#include "../imu_sender/imu_packet.h"
#include <cassert>
#include <cstring>

int main() {
  const uint8_t data[] = {
      0x40, 0x00, 0xc0, 0x00, 0x7f, 0xff, 0x12, 0x34,
      0x00, 0x83, 0xff, 0x7d, 0x80, 0x00};
  char packet[96];
  const int length = formatImuPacket(packet, sizeof(packet), 42, 12345, data);
  assert(strcmp(packet, "IMU,42,12345,16384,-16384,32767,131,-131,-32768") == 0);
  assert(length == static_cast<int>(strlen(packet)));
  char shortPacket[4];
  assert(formatImuPacket(shortPacket, sizeof(shortPacket), 42, 12345, data) >=
         static_cast<int>(sizeof(shortPacket)));
}
