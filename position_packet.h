#ifndef POSITION_PACKET_H
#define POSITION_PACKET_H

#include <math.h>
#include <stdlib.h>
#include <string.h>

// POS,latitude_degrees,longitude_degrees,altitude_metres
inline bool isPositionPacket(const char *packet) {
  if (strncmp(packet, "POS,", 4) != 0) {
    return false;
  }

  const char *value = packet + 4;
  for (int field = 0; field < 3; ++field) {
    const char *start = value;
    while ((*value >= '0' && *value <= '9') || *value == '-' ||
           *value == '+' || *value == '.' || *value == 'e' || *value == 'E') {
      ++value;
    }

    char *end;
    const double number = strtod(start, &end);
    if (end == start || end != value || !isfinite(number) ||
        (field == 0 && (number < -90 || number > 90)) ||
        (field == 1 && (number < -180 || number > 180)) ||
        (field < 2 && *value != ',') ||
        (field == 2 && *value != '\0')) {
      return false;
    }
    if (field < 2) {
      ++value;
    }
  }
  return true;
}

#endif
