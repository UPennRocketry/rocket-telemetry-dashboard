#ifndef IMU_PACKET_H
#define IMU_PACKET_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

inline int16_t imuSignedWord(const uint8_t *bytes) {
  const int32_t value = (static_cast<int32_t>(bytes[0]) << 8) | bytes[1];
  return static_cast<int16_t>(value >= 32768 ? value - 65536 : value);
}

// MPU6050 burst order: accel XYZ, temperature, gyro XYZ.
inline int formatImuPacket(char *buffer, size_t size, uint32_t sequence,
                           uint32_t uptimeMs, const uint8_t *data) {
  return snprintf(buffer, size, "IMU,%lu,%lu,%d,%d,%d,%d,%d,%d",
                  static_cast<unsigned long>(sequence),
                  static_cast<unsigned long>(uptimeMs),
                  static_cast<int>(imuSignedWord(data)),
                  static_cast<int>(imuSignedWord(data + 2)),
                  static_cast<int>(imuSignedWord(data + 4)),
                  static_cast<int>(imuSignedWord(data + 8)),
                  static_cast<int>(imuSignedWord(data + 10)),
                  static_cast<int>(imuSignedWord(data + 12)));
}

#endif
