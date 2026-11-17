#ifndef FLIGHT_MODEL_H
#define FLIGHT_MODEL_H

#include <math.h>
#include <stdint.h>

// Matches the standalone dashboard's Pathfinder II simulated flight model.
constexpr float BURN = 1.8f, APO = 14.6f, MAIN = 42.1f, LAND = 127.3f;
constexpr float APO_ALT = 3386.0f, BURN_ALT = 620.0f, MAIN_ALT = 1874.0f;
constexpr float DOWNRANGE = 2220.0f, TOP_MPH = 284.0f, MAX_G = 12.2f;
constexpr int32_t FIRST_FLIGHT_MS = -12000, LAST_FLIGHT_MS = 132000;
constexpr size_t FLIGHT_SAMPLE_COUNT = (LAST_FLIGHT_MS - FIRST_FLIGHT_MS) / 1000 + 1;

struct FlightSample {
  int32_t timeMs;
  long altitudeFt;
  long downrangeFt;
  long velocityTenthsMph;
  long accelerationHundredthsG;
};

inline FlightSample flightSample(size_t index) {
  const int32_t timeMs = FIRST_FLIGHT_MS + static_cast<int32_t>(index) * 1000;
  const float t = timeMs / 1000.0f;
  float altitude = 0, downrange = 0, velocity = 0, acceleration = 1;
  if (t > 0 && t < BURN) {
    altitude = BURN_ALT * powf(t / BURN, 1.85f);
    downrange = 60 * powf(t / BURN, 2);
    velocity = TOP_MPH * (t / BURN);
    acceleration = 1 + (MAX_G - 1) * sinf(3.14159265358979323846f * t / BURN);
  } else if (t >= BURN && t < APO) {
    altitude = APO_ALT - (APO_ALT - BURN_ALT) * powf((APO - t) / (APO - BURN), 2);
    downrange = 60 + 460 * powf((t - BURN) / (APO - BURN), 1.3f);
    velocity = TOP_MPH * ((APO - t) / (APO - BURN));
    acceleration = 0.9f * ((APO - t) / (APO - BURN));
  } else if (t >= APO && t < MAIN) {
    altitude = APO_ALT - 55 * (t - APO);
    downrange = 520 + 700 * ((t - APO) / (MAIN - APO));
    velocity = 37;
  } else if (t >= MAIN && t < LAND) {
    altitude = MAIN_ALT - 22 * (t - MAIN);
    downrange = 1220 + 1000 * ((t - MAIN) / (LAND - MAIN));
    velocity = 15;
  } else if (t >= LAND) {
    downrange = DOWNRANGE;
  }
  return {timeMs, lroundf(altitude), lroundf(downrange),
          lroundf(velocity * 10), lroundf(acceleration * 100)};
}

#endif
