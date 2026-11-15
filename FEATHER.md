# Feather M0 LoRa position link

`feather.ino` listens using the Feather M0's built-in RFM9x radio and prints
each received LoRa packet's payload bytes and RSSI to USB serial at 115200 baud.
It also recognizes the position format below and prints a labeled position.
It does not transmit a reply. The receiver waits until a serial monitor connects
before initializing the radio. `friend_sender/friend_sender.ino` is a matching
transmitter sketch for a second Feather M0 with an RFM9x radio.

The configured radio settings are **433 MHz**, **125 kHz bandwidth**, **SF7**,
**coding rate 4/5**, **sync word 0x12**, and explicit packet headers. These are
starting settings; both radios must use compatible settings. This changes the
receiver's previous 434 MHz setting. Make sure both radio modules and antennas
support the chosen frequency, and follow applicable local radio rules and
launch-site coordination. This is a direct radio link, not an internet link.

The sender does **not** read an altimeter or GPS by itself. A person or a
program on your friend's laptop must supply the actual readings over USB
serial. The Featherweight altimeter's connection and data format are not known,
so no automatic Featherweight integration is included. Do not use this link as
the sole means of tracking or recovering a rocket: packets can be lost, and
there is no acknowledgement, authentication, timestamp, or stale-data check.

The receiver gets complete LoRa payloads, not arbitrary RF signals. RadioHead
packets include their RadioHead header bytes in the printed payload. Binary
payloads may include non-printable characters; LoRaWAN traffic may be encrypted.

## Position format

Send one newline-terminated line per reading, with decimal degrees and altitude
in metres:

```text
POS,39.952583,-75.165222,123.45
```

Latitude must be between -90 and 90; longitude between -180 and 180. Altitude
must be a finite number. Agree on the altitude reference (for example GPS
altitude above mean sea level) with the data source; the format does not encode
it. Do not send a position if there is no valid GPS fix. The sender forwards each
nonempty line as a packet, up to 255 bytes (use at most 254 bytes with CRLF line
endings). It does not validate the fields; the receiver validates recognized
`POS,` packets and reports malformed ones. Other packets are still printed raw.
The sender's "Packet sent" message means the radio transmitted, not that the
other Feather received it.

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
cp position_packet.h "$SKETCH_DIR/position_packet.h"
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

## Flash the sender

On your friend's laptop, install the same tools and library above. From the
repository root, connect her Feather and use its actual port:

```sh
arduino-cli board list
SENDER_BUILD="$(mktemp -d)"
arduino-cli compile --fqbn adafruit:samd:adafruit_feather_m0 --build-path "$SENDER_BUILD" friend_sender &&
  arduino-cli upload -p /dev/cu.usbmodem1201 --fqbn adafruit:samd:adafruit_feather_m0 --build-path "$SENDER_BUILD" friend_sender
```

These commands use a macOS/Linux shell. On Windows, use an appropriate build
directory and the `COM` port shown by `arduino-cli board list` instead.
Both sketches wait for their USB serial connections before initializing the
radio. Keep both laptops connected during this test.

On her laptop, open Arduino IDE's Serial Monitor at 115200 baud with **Newline**
line ending, then enter and send the example position above. Alternatively, a
program that can read her data can write lines in that format to the sender's
USB serial port at 115200 baud. Only one program can normally open that port at
a time. Do not transmit test coordinates as if they were real readings.

## Monitor

Open the Feather's current port in a terminal:

```sh
screen /dev/cu.usbmodem1201 115200
```

Replace the port if needed. To exit `screen`, press **Ctrl+A**, then **K**, then
**Y**. You should see the initialization message; press reset if you opened the
monitor after it appeared. When the sender transmits the example, the receiver
prints the raw packet, RSSI, and a labeled position. With no transmitting
device, the receiver simply waits. Neither sketch updates the dashboard.
