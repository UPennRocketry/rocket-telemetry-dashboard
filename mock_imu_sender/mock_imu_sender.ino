#include <SPI.h>
#include <LoRa.h>
#include "flight_model.h"

constexpr int RFM95_CS = 8;
constexpr int RFM95_RST = 4;
constexpr int RFM95_INT = 3;
constexpr long RADIO_FREQUENCY_HZ = 433000000;
constexpr long RADIO_BANDWIDTH_HZ = 125000;
constexpr int RADIO_SPREADING_FACTOR = 7;
constexpr int RADIO_CODING_RATE = 5;
constexpr int RADIO_SYNC_WORD = 0x12;
constexpr unsigned long SEND_INTERVAL_MS = 1000;

uint32_t sequence = 0;
size_t sampleIndex = 0;
unsigned long lastAttempt = 0;

void setup() {
  Serial.begin(115200);
  LoRa.setPins(RFM95_CS, RFM95_RST, RFM95_INT);
  while (!LoRa.begin(RADIO_FREQUENCY_HZ)) {
    Serial.println("ERROR: Radio initialization failed; retrying in 5 seconds.");
    delay(5000);
  }
  LoRa.setSignalBandwidth(RADIO_BANDWIDTH_HZ);
  LoRa.setSpreadingFactor(RADIO_SPREADING_FACTOR);
  LoRa.setCodingRate4(RADIO_CODING_RATE);
  LoRa.setSyncWord(RADIO_SYNC_WORD);
  Serial.println("SIMULATED flight sender started on 433 MHz; no sensor required.");
  lastAttempt = millis() - SEND_INTERVAL_MS;
}

void loop() {
  const unsigned long now = millis();
  if (now - lastAttempt < SEND_INTERVAL_MS) {
    return;
  }
  lastAttempt = now;

  char packet[128];
  const FlightSample sample = flightSample(sampleIndex);
  const int length = snprintf(packet, sizeof(packet), "SIM_FLIGHT,%lu,%ld,%ld,%ld,%ld,%ld",
                              static_cast<unsigned long>(sequence),
                              static_cast<long>(sample.timeMs),
                              sample.altitudeFt, sample.downrangeFt,
                              sample.velocityTenthsMph, sample.accelerationHundredthsG);
  if (length < 0 || static_cast<size_t>(length) >= sizeof(packet)) {
    Serial.println("ERROR: Simulated flight packet formatting failed.");
    return;
  }
  if (!LoRa.beginPacket()) {
    Serial.println("ERROR: Radio busy; simulated flight packet not sent.");
    return;
  }
  const size_t written = LoRa.write(reinterpret_cast<const uint8_t *>(packet), length);
  const int sent = LoRa.endPacket();
  if (written != static_cast<size_t>(length) || sent != 1) {
    Serial.println("ERROR: Simulated flight packet transmission failed.");
    return;
  }
  Serial.print("Sent: ");
  Serial.println(packet);
  ++sequence;
  sampleIndex = (sampleIndex + 1) % FLIGHT_SAMPLE_COUNT;
}
