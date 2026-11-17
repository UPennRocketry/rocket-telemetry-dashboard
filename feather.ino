#include <SPI.h>
#include <LoRa.h>
#include "position_packet.h"

// Feather M0 with the built-in RFM9x radio.
constexpr int RFM95_CS = 8;
constexpr int RFM95_RST = 4;
constexpr int RFM95_INT = 3;

// These settings must match the transmitter.
constexpr long RADIO_FREQUENCY_HZ = 433000000;
constexpr long RADIO_BANDWIDTH_HZ = 125000;
constexpr int RADIO_SPREADING_FACTOR = 7;
constexpr int RADIO_CODING_RATE = 5;  // 4/5
constexpr int RADIO_SYNC_WORD = 0x12;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println("Feather M0 LoRa receiver");
  LoRa.setPins(RFM95_CS, RFM95_RST, RFM95_INT);
  if (!LoRa.begin(RADIO_FREQUENCY_HZ)) {
    Serial.println("ERROR: Radio initialization failed. Check the board and radio pins.");
    while (true) {
      delay(1000);
    }
  }

  LoRa.setSignalBandwidth(RADIO_BANDWIDTH_HZ);
  LoRa.setSpreadingFactor(RADIO_SPREADING_FACTOR);
  LoRa.setCodingRate4(RADIO_CODING_RATE);
  LoRa.setSyncWord(RADIO_SYNC_WORD);
  Serial.println("Listening on 433 MHz, BW 125 kHz, SF7, CR 4/5, sync 0x12.");
  Serial.println("Waiting for LoRa packets...");
}

void loop() {
  const int packetSize = LoRa.parsePacket();
  if (packetSize == 0) {
    return;
  }

  Serial.print("\nPacket received (");
  Serial.print(packetSize);
  Serial.print(" bytes, RSSI ");
  Serial.print(LoRa.packetRssi());
  Serial.println(" dBm):");
  char payload[256];
  int length = 0;
  bool textPacket = true;
  while (LoRa.available() && length < 255) {
    const uint8_t byte = static_cast<uint8_t>(LoRa.read());
    payload[length++] = static_cast<char>(byte);
    if ((byte < 0x20 && byte != '\r' && byte != '\n' && byte != '\t') ||
        byte > 0x7e) {
      textPacket = false;
    }
  }
  payload[length] = '\0';

  if (textPacket) {
    Serial.write(reinterpret_cast<const uint8_t *>(payload), length);
    Serial.println();
  } else {
    Serial.print("Binary payload (hex): ");
    for (int i = 0; i < length; ++i) {
      const uint8_t byte = static_cast<uint8_t>(payload[i]);
      if (byte < 0x10) {
        Serial.print('0');
      }
      Serial.print(byte, HEX);
      if (i + 1 < length) {
        Serial.print(' ');
      }
    }
    Serial.println();
  }

  if (textPacket && isPositionPacket(payload)) {
    Serial.print("Position (latitude degrees, longitude degrees, altitude metres): ");
    Serial.println(payload + 4);
  } else if (textPacket && strncmp(payload, "POS,", 4) == 0) {
    Serial.println("ERROR: Invalid position packet.");
  }
}
