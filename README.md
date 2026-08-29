# TankSync

TankSync is a wireless water-tank monitoring system built around ESP32 and LoRa.

The system uses a low-power transmitter installed near the water tank to measure tank level and transmitter battery status. The measurements are transmitted wirelessly to an ESP32-based receiver, which processes the telemetry and displays the current tank and battery status on an OLED display.

The project is designed with a focus on:

* Low-power operation
* Long-range wireless communication
* Simple and reliable telemetry
* Expandability to multiple tanks/transmitters
* Maintainable firmware architecture
* A simple local display without requiring Wi-Fi or cloud services

---

## System Overview

```text
┌─────────────────────┐
│      TANK 1         │
│                     │
│  Distance Sensor    │
│        │            │
│        ▼            │
│   ┌───────────┐     │
│   │Transmitter│     │
│   │   ESP32   │     │
│   └─────┬─────┘     │
└─────────┼───────────┘
          │
          │ LoRa
          │
          ▼
   ┌───────────────┐
   │   Receiver    │
   │     ESP32     │
   └───────┬───────┘
           │
           ▼
      ┌─────────┐
      │  OLED   │
      │ Display │
      └─────────┘
```

The transmitter periodically measures the tank and sends a telemetry packet.

The receiver:

1. Receives the LoRa packet.
2. Validates the packet.
3. Identifies the transmitter.
4. Maps the transmitter to its configured tank.
5. Calculates tank percentage and volume.
6. Calculates battery percentage.
7. Displays the processed information.

---

# Current Status

🚧 **Project under active development**

The current receiver implementation supports:

* ESP32
* RYLR998 LoRa module
* SH1106 128×64 OLED
* Tank level calculation
* Battery voltage and percentage
* Charging status
* RSSI
* SNR
* Tank/transmitter configuration tables
* Versioned TankSync application packets
* Packet validation
* OLED tank screen
* OLED battery screen
* Button-based screen switching
* Animated level transitions
* LoRa connection timeout handling

The transmitter is being developed alongside the receiver with low-power operation and deep-sleep operation as a major design goal.

---

# Repository Structure

```text
TankSync/
│
├── transmitter/
│   └── Transmitter firmware
│
├── receiver/
│   └── Receiver firmware
│
├── docs/
│   ├── architecture.md
│   ├── protocol.md
│   ├── hardware.md
│   └── development.md
│
└── README.md
```

The transmitter and receiver are intentionally kept in the same repository because they are two parts of the same system and share the TankSync application protocol.

---

# Firmware Architecture

Both firmware projects are organized into independent modules rather than keeping the entire application in a single Arduino `.ino` file.

## Receiver

```text
receiver/
│
├── receiver.ino
│
├── config/
│   ├── BoardConfig.h
│   ├── LoRaConfig.h
│   ├── PacketConfig.h
│   └── TankConfig.h
│
├── models/
│   ├── Telemetry.h
│   └── DisplayData.h
│
├── lora/
│   ├── LoRaManager.h
│   └── LoRaManager.cpp
│
├── protocol/
│   ├── PacketParser.h
│   └── PacketParser.cpp
│
├── tank/
│   ├── TankProcessor.h
│   └── TankProcessor.cpp
│
├── display/
│   ├── DisplayManager.h
│   └── DisplayManager.cpp
│
└── input/
    ├── ButtonManager.h
    └── ButtonManager.cpp
```

The main `.ino` file acts primarily as the application coordinator.

The major responsibilities are separated into:

```text
LoRaManager
    ↓
PacketParser
    ↓
TankProcessor
    ↓
DisplayManager
```

This makes changes localized.

For example:

| Change                         | File                         |
| ------------------------------ | ---------------------------- |
| ESP32 GPIO                     | `config/BoardConfig.h`       |
| LoRa settings                  | `config/LoRaConfig.h`        |
| Packet requirements            | `config/PacketConfig.h`      |
| Tank/transmitter configuration | `config/TankConfig.h`        |
| Packet format                  | `protocol/PacketParser.cpp`  |
| Tank calculations              | `tank/TankProcessor.cpp`     |
| OLED layout                    | `display/DisplayManager.cpp` |
| Button behavior                | `input/ButtonManager.cpp`    |

---

# Communication

TankSync uses a two-layer packet structure.

The RYLR998 provides the radio transport envelope:

```text
+RCV=<address>,<length>,<data>,<RSSI>,<SNR>
```

Inside the LoRa payload is the TankSync application packet:

```text
TS|v=1|id=1|seq=123|dist=43.2|bat=3.87|chg=0
```

Where:

| Field  | Meaning                 |
| ------ | ----------------------- |
| `TS`   | TankSync packet header  |
| `v`    | Protocol version        |
| `id`   | Transmitter ID          |
| `seq`  | Packet sequence number  |
| `dist` | Measured distance in cm |
| `bat`  | Battery voltage         |
| `chg`  | Charging state          |

The protocol is designed so that fields can evolve without requiring the receiver to be rewritten for every additional field.

Unknown fields can be ignored, while configured required fields must be present.

More details are documented in:

`docs/protocol.md`

---

# Tank Level Calculation

The receiver does not directly interpret sensor distance as a percentage.

Each tank has its own calibrated measurement range.

For example:

```text
Sensor
  │
  │ 10 cm
  ▼
┌───────────┐
│   FULL    │ 100%
│           │
│           │
│           │
│           │
│   EMPTY   │   0%
└───────────┘
  │
  │ 95 cm
  ▼
```

The tank configuration defines:

* Tank ID
* Tank length
* Tank capacity
* Full distance
* Empty distance

The transmitter configuration defines:

* Transmitter ID
* Associated tank
* Sensor minimum range
* Sensor maximum range
* Battery full voltage
* Battery empty voltage

This allows multiple tanks and transmitters with different physical characteristics to be supported.

---

# Hardware

## Receiver

The current receiver uses:

* ESP32
* RYLR998 LoRa module
* 1.3" SH1106 128×64 monochrome OLED
* Push button

Current receiver GPIO configuration is maintained in:

```text
receiver/config/BoardConfig.h
```

## Transmitter

The transmitter hardware and pin configuration are maintained separately under:

```text
transmitter/
```

Refer to:

`docs/hardware.md`

for the current wiring and hardware information.

---

# Software Requirements

The firmware is intended to be built using the Arduino ecosystem for ESP32.

Required libraries for the current receiver include:

* ESP32 Arduino core
* U8g2
* Wire

The LoRa module communicates with the ESP32 through UART.

Install the required libraries through the Arduino IDE before compiling.

---

# Building

## Receiver

Open:

```text
receiver/receiver.ino
```

in the Arduino IDE.

Select the appropriate ESP32 board and serial port, then compile and upload.

## Transmitter

Open:

```text
transmitter/transmitter.ino
```

and compile/upload using the appropriate ESP32 board configuration.

---

# Configuration

Hardware and application configuration is intentionally kept separate from application logic.

Before deploying a receiver, review:

```text
receiver/config/BoardConfig.h
receiver/config/LoRaConfig.h
receiver/config/PacketConfig.h
receiver/config/TankConfig.h
```

For example, adding another transmitter should primarily require updating the transmitter configuration table rather than changing the processing logic.

---

# Power Management

Low-power operation is an important part of the TankSync design.

The transmitter is intended to spend most of its time in ESP32 deep sleep and periodically wake up to:

1. Wake the ESP32.
2. Initialize the required hardware.
3. Measure the tank.
4. Read battery information.
5. Transmit telemetry.
6. Return to deep sleep.

The receiver is expected to remain powered continuously.

Power-management behavior is under active development.

---

# Design Principles

TankSync follows several design principles.

### 1. Separate configuration from logic

Physical configuration should not be scattered throughout the firmware.

### 2. Separate transport from application protocol

RYLR998 communication and TankSync packet parsing are treated as different layers.

### 3. Keep hardware-specific code isolated

OLED, button, LoRa and sensor handling should not leak into unrelated modules.

### 4. Keep calculations independent from the UI

Tank and battery calculations should produce data models that the display can consume.

### 5. Prefer simple embedded-friendly architecture

The project should remain understandable and lightweight enough for ESP32-class hardware.

### 6. Avoid unnecessary dependencies

The project should not require cloud services or a network connection for its core functionality.

---

# Future Direction

Potential future improvements include:

* ESP32 deep-sleep transmitter
* Periodic hourly telemetry
* Persistent last-known telemetry on receiver
* Multiple transmitters
* Multiple tanks
* Improved stale-data handling
* Better transmitter/receiver synchronization
* Battery optimization
* Improved diagnostics
* OTA firmware updates
* Additional display information

These features should be introduced without unnecessarily coupling the transmitter and receiver implementations.

---

# Development

When modifying the project, prefer changing the smallest appropriate module.

For example:

```text
Change OLED layout
    → receiver/display/

Change packet format
    → receiver/protocol/

Change tank calculation
    → receiver/tank/

Change LoRa configuration
    → receiver/config/

Change GPIO
    → receiver/config/

Change transmitter sleep behavior
    → transmitter/

Change shared communication protocol
    → protocol-related files
```

Avoid putting unrelated functionality into `receiver.ino` or `transmitter.ino`.

---

# Repository Status

This repository represents the development version of TankSync.

Hardware, packet formats and firmware behavior may change as the project evolves.

Do not treat the current protocol as permanently frozen unless a specific protocol version is marked as stable.

---

# License

Add the project's chosen license here.

For example:

```text
MIT License
```

if the project is intended to be released under the MIT license.
