# Feather M0 LoRa receiver

`feather.ino` listens using the Feather M0's built-in RFM9x radio and prints
each received LoRa packet's payload bytes and RSSI to USB serial at 115200 baud.
It does not transmit a reply. The receiver waits until a serial monitor connects
before initializing the radio.

The configured radio settings are **434 MHz**, **125 kHz bandwidth**, **SF7**,
**coding rate 4/5**, **sync word 0x12**, and explicit packet headers. These are
starting settings; the future transmitter must use compatible settings. The
sketch receives complete LoRa payloads, not arbitrary RF signals. RadioHead
packets include their RadioHead header bytes in the printed payload. Binary
payloads may include non-printable characters; LoRaWAN traffic may be encrypted.

## Install tools (macOS)

With Homebrew installed, run:

```sh
brew install arduino-cli
arduino-cli core update-index --additional-urls https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
arduino-cli core install arduino:samd adafruit:samd --additional-urls https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
arduino-cli lib install LoRa
```

The Adafruit core and its toolchain require available disk space. If an install
fails with `no space left on device`, free space and rerun the install commands.

## Compile and flash

From the repository root, with the Feather connected over USB:

```sh
arduino-cli board list
WORK_DIR="$(mktemp -d)"
SKETCH_DIR="$WORK_DIR/feather"
mkdir -p "$SKETCH_DIR"
cp feather.ino "$SKETCH_DIR/feather.ino"
arduino-cli compile --fqbn adafruit:samd:adafruit_feather_m0 --build-path "$WORK_DIR/build" "$SKETCH_DIR" &&
  arduino-cli upload -p /dev/cu.usbmodem1201 --fqbn adafruit:samd:adafruit_feather_m0 --build-path "$WORK_DIR/build" "$SKETCH_DIR"
```

Replace `/dev/cu.usbmodem1201` with the Feather's current port from
`arduino-cli board list`; USB device names can change. Close any serial monitor
before uploading. If automatic upload fails, double-press the Feather's reset
button to enter its bootloader, find the new port, and retry the upload using
that port. The normal serial port can change again after upload.
The temporary directory's path is printed by `printf '%s\n' "$WORK_DIR"` if you
want to remove it after the upload.

## Monitor

Open the Feather's current port in a terminal:

```sh
screen /dev/cu.usbmodem1201 115200
```

Replace the port if needed. To exit `screen`, press **Ctrl+A**, then **K**, then
**Y**. You should see the initialization message; press reset if you opened the
monitor after it appeared. With only an antenna wire and no transmitting
device, the receiver will simply wait and no packets are expected.
