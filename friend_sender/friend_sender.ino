#include <SPI.h>
#include <LoRa.h>

constexpr int RFM95_CS = 8;
constexpr int RFM95_RST = 4;
constexpr int RFM95_INT = 3;
constexpr long RADIO_FREQUENCY_HZ = 433000000;
constexpr long RADIO_BANDWIDTH_HZ = 125000;
constexpr int RADIO_SPREADING_FACTOR = 7;
constexpr int RADIO_CODING_RATE = 5;
constexpr int RADIO_SYNC_WORD = 0x12;

char line[256];
size_t length = 0;
bool overflow = false;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

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
  Serial.println("Sender ready. Enter POS,latitude,longitude,altitude_metres and a newline.");
}

void loop() {
  while (Serial.available()) {
    const char byte = static_cast<char>(Serial.read());
    if (byte == '\n') {
      if (overflow) {
        Serial.println("ERROR: Input exceeds 255 bytes.");
      } else if (length > 0) {
        if (line[length - 1] == '\r') {
          --length;
        }
        if (length > 0) {
          if (!LoRa.beginPacket()) {
            Serial.println("ERROR: Radio is busy; packet not sent.");
          } else {
            LoRa.write(reinterpret_cast<const uint8_t *>(line), length);
            if (LoRa.endPacket() != 1) {
              Serial.println("ERROR: Radio transmission failed.");
            } else {
              Serial.println("Packet sent; receiver validates position data.");
            }
          }
        }
      }
      length = 0;
      overflow = false;
    } else if (!overflow) {
      if (length == sizeof(line) - 1) {
        overflow = true;
      } else {
        line[length++] = byte;
      }
    }
  }
}
