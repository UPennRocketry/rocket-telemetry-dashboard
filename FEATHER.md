# Feather M0 LoRa links

## Simulated rocket-flight sender (no IMU needed)

Flash `mock_imu_sender/mock_imu_sender.ino` to the sender and `feather.ino`
with `position_packet.h` to the receiver. The sender calculates the same
illustrative flight model as the dashboard, sends CSV packets once per
second, and repeats after 145 samples. No sensor, SD card, or sender serial
monitor is needed. The receiver prints the packets and RSSI.
Use the 433 MHz radio/antenna and legal precautions described below. These
synthetic values are not flight data or a physics simulation and must not be
used for actual flight decisions.

The packet format is CSV:

```text
SIM_FLIGHT,sequence,mission_time_ms,altitude_ft,downrange_ft,speed_tenths_mph,acceleration_hundredths_g
SIM_FLIGHT,0,-12000,0,0,0,100
```

`SIM_FLIGHT` identifies the readings as simulated, distinct from real `IMU`
packets. Sequence increases across cycles and resets when the sender resets;
mission time restarts at -12 seconds on each cycle. Apogee and deployment
are model events, not commands to hardware. The sender's `Sent` message
does not confirm reception.

From the repository root, flash the sender first. Replace the example port
with the sender's port from `arduino-cli board list`:

```sh
arduino-cli board list
SENDER_PORT=/dev/cu.usbmodem1201
SENDER_BUILD="$(mktemp -d)"
arduino-cli compile --fqbn adafruit:samd:adafruit_feather_m0 --build-path "$SENDER_BUILD" mock_imu_sender &&
  arduino-cli upload -p "$SENDER_PORT" --fqbn adafruit:samd:adafruit_feather_m0 --build-path "$SENDER_BUILD" mock_imu_sender
```

Then connect the receiver and use its actual port:

```sh
arduino-cli board list
RECEIVER_PORT=/dev/cu.usbmodem1201
WORK_DIR="$(mktemp -d)"
mkdir -p "$WORK_DIR/feather"
cp feather.ino position_packet.h "$WORK_DIR/feather/"
arduino-cli compile --fqbn adafruit:samd:adafruit_feather_m0 --build-path "$WORK_DIR/build" "$WORK_DIR/feather" &&
  arduino-cli upload -p "$RECEIVER_PORT" --fqbn adafruit:samd:adafruit_feather_m0 --build-path "$WORK_DIR/build" "$WORK_DIR/feather"
```

With the sender powered and the receiver connected, find the receiver's
current port again and open it at 115200 baud for a serial-only check:

```sh
arduino-cli board list
screen /dev/cu.usbmodem1201 115200
```

Replace the example port; exit with Ctrl+A, K, Y. The receiver waits for
the monitor before starting its radio. The sender's monitor at 115200 baud
shows transmitted packets and errors; close a monitor before uploading.
Both sketches assume Feather M0 boards with built-in 433 MHz RFM9x radios.
For the visual dashboard instead of `screen`, close the serial monitor and
follow [the dashboard instructions](web_dashboard/README.md). Only one
application can open the receiver's serial port at a time.

## Automatic MPU6050 sender

Use `imu_sender/imu_sender.ino` (and its adjacent `imu_packet.h`) on the
sender, and `feather.ino` with `position_packet.h` on the receiver. The
receiver already prints arbitrary packets, so it also prints IMU packets.
The older `friend_sender` sketch below is a separate manual position sender.

These instructions assume **both** boards are Feather M0s with built-in
RFM9x LoRa radios made for 433 MHz, and the sensor is an MPU6050 I2C breakout.
433 MHz is the radio carrier frequency, not the sample rate. Both radios
need suitable 433 MHz antennas connected before the sender is powered.
Check your local frequency, power, duty-cycle, and launch-site rules before
transmitting; the configured frequency and interval are not a guarantee of
legal operation everywhere.

### Wire the sensor to the sender

With power disconnected, connect:

| MPU6050 breakout | Sender Feather M0 |
| --- | --- |
| VCC / VIN | 3V / 3V3 |
| GND | GND |
| SDA | SDA |
| SCL | SCL |

Use the Feather's 3.3 V supply and 3.3 V I2C logic, not USB/5 V. Check
the breakout's labels and documentation: a bare MPU6050 must not receive 5 V,
and any SDA/SCL pull-ups must go to 3.3 V. I2C needs pull-up resistors;
many breakouts already provide them. AD0 selects address 0x68 when low and
0x69 when high; the sender checks both. If AD0 is exposed and not already
strapped on the breakout, connect it to GND; do not fight an existing strap.
INT is not used. No sensor is needed on the receiver.

### Flash and monitor

Install the tools and LoRa library from **Install tools (macOS)** below.
From the repository root, connect only the receiver first, find its port and
compile/upload it:

```sh
arduino-cli board list
RECEIVER_PORT=/dev/cu.usbmodem1201  # replace with the receiver's actual port
WORK_DIR="$(mktemp -d)"
mkdir -p "$WORK_DIR/feather"
cp feather.ino position_packet.h "$WORK_DIR/feather/"
arduino-cli compile --fqbn adafruit:samd:adafruit_feather_m0 --build-path "$WORK_DIR/build" "$WORK_DIR/feather" &&
  arduino-cli upload -p "$RECEIVER_PORT" --fqbn adafruit:samd:adafruit_feather_m0 --build-path "$WORK_DIR/build" "$WORK_DIR/feather"
```

Disconnect it, connect the sender, and use its actual port:

```sh
arduino-cli board list
SENDER_PORT=/dev/cu.usbmodem1201  # replace with the sender's actual port
SENDER_BUILD="$(mktemp -d)"
arduino-cli compile --fqbn adafruit:samd:adafruit_feather_m0 --build-path "$SENDER_BUILD" imu_sender &&
  arduino-cli upload -p "$SENDER_PORT" --fqbn adafruit:samd:adafruit_feather_m0 --build-path "$SENDER_BUILD" imu_sender
```

Close serial monitors before uploading. USB port names can change; if upload
fails, double-press reset for the bootloader, find its new port and retry.
After flashing, reconnect the receiver and check `arduino-cli board list`
again. With the sender powered, open the receiver's current port:

```sh
screen /dev/cu.usbmodem1201 115200
```

Replace the example port. Exit with **Ctrl+A**, then **K**, then **Y**.
The receiver waits for a serial connection before starting its radio; press
reset if needed. The sender does not wait for USB and automatically starts
transmitting on power. It tries immediately, then every 1 second
(`SEND_INTERVAL_MS`); change that constant only after checking radio rules.
If the sensor is missing or a read fails, it skips the packet and retries.
For diagnostics, connect a serial monitor to the sender at 115200 baud;
press reset to see startup messages. Sender-side errors cannot appear on the
receiver if no packet is sent.

On initialization, the sender reports whether I2C addresses 0x68 and 0x69
acknowledge, and prints `WHO_AM_I` for any responding device. A no-ACK or
bus-error status points to an I2C communication problem, not necessarily
incorrect wiring. A successful ACK with an unexpected identity indicates
that a device responded but did not identify itself as the expected MPU6050.
The sensor LED indicates power, not that I2C communication works.

The receiver prints the packet and RSSI, for example:

```text
IMU,0,1234,100,-200,16384,10,-20,30
```

The fields are `IMU,sequence,uptime_ms,ax,ay,az,gx,gy,gz`. Sequence starts
at zero on sender reset and counts locally successful transmissions, not
confirmed receptions; uptime is sender milliseconds and wraps. Acceleration
values are signed raw counts at +/-2 g (divide by 16384 for g); angular
velocity values are signed raw counts at +/-250 degrees/s (divide by 131
for degrees/s). Acceleration includes gravity. These are uncalibrated sensor
readings, not orientation or position. Packet loss is possible, and neither
sketch feeds the dashboard. Do not rely on this link as the sole means of
tracking or recovering a rocket.

If the receiver shows a binary payload instead of `IMU,...`, that is not a
valid IMU text packet. The receiver listens to any matching LoRa traffic; a
different transmitter, corrupted data, or firmware mismatch could be the
cause. It prints non-text bytes as hexadecimal, not decoded IMU values.
Turn off the sender briefly to see whether such packets continue. Then
reflash the intended sender, connect its USB serial monitor at 115200 baud,
and verify it prints `Sent: IMU,...` once per second. A sensor error means
no IMU packet is sent; check the wiring above. A successful sender print
means transmission, not confirmed reception. The current IMU sender does not
enable payload CRC, so a packet alone does not establish its integrity.

## Manual position sender

`feather.ino` listens using the Feather M0's built-in RFM9x radio and prints
each received LoRa packet's payload and RSSI to USB serial at 115200 baud.
Printable payloads are shown as text; non-text payloads are shown as hex.
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
packets include their RadioHead header bytes in the printed payload. LoRaWAN
traffic may be encrypted.

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
