#include <SPI.h>
#include <LoRa.h>

// Feather M0 with the built-in RFM9x radio.
constexpr int RFM95_CS = 8;
constexpr int RFM95_RST = 4;
constexpr int RFM95_INT = 3;

// These settings must match the transmitter.
constexpr long RADIO_FREQUENCY_HZ = 434000000;
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
  Serial.println("Listening on 434 MHz, BW 125 kHz, SF7, CR 4/5, sync 0x12.");
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
  while (LoRa.available()) {
    Serial.write(static_cast<uint8_t>(LoRa.read()));
  }
  Serial.println();
}
