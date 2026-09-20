# DASS HOME

DASS HOME is a wireless water-tank monitoring system built around ESP32 and LoRa.

The system uses a low-power transmitter installed near the water tank to measure tank level and transmitter battery status. The measurements are transmitted wirelessly to an ESP32-based receiver, which processes the telemetry and displays the current tank and battery status on an OLED display.

The project is designed with a focus on:

* Low-power operation
* Long-range wireless communication
* Simple and reliable telemetry
* Expandability to multiple tanks/transmitters
* Maintainable firmware architecture
* Wi-Fi captive setup with OLED and local IP display

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

1. Connects to Wi-Fi (or provides a captive setup portal with IP on OLED).
2. Receives the LoRa packet.
3. Validates the packet.
4. Identifies the transmitter.
5. Maps the transmitter to its configured tank.
6. Calculates tank percentage and volume.
7. Calculates battery percentage.
8. Displays the processed information.

---

# Current Status

🚧 **Project under active development**

The current receiver implementation supports:

* **Hardware & RF**:
  * ESP32 microcontroller
  * RYLR998 LoRa transceiver via HardwareSerial UART
  * 1.3" SH1106 128×64 I2C OLED display (U8g2)
  * INA219 I2C high-side voltage, current, and power sensor
  * 5-LED hardware fuel gauge with charging animations
  * Single multi-function button (wake, page cycle, section selection)
* **Telemetry & Storage**:
  * Versioned DASS HOME application packet validation (`TS|v=1|...`)
  * Dynamic tank level and volume calculations
  * Transmitter battery voltage and percentage calculation
  * Persistent NVS telemetry caching across reboots
  * RSSI and SNR signal quality tracking
  * Optional mock data generator for testing without LoRa hardware
* **Time & UI**:
  * NTP time synchronization with configurable servers and timezone offsets
  * Dynamic multi-section OLED UI (Tank, Transmitter Battery, Config, Dev Mode)
  * Live elapsed time footer ("updated Xm ago")
  * Inactivity auto-sleep (configurable timeout) and instant button wake
  * Charging detection splash screen and animated hourglass waiting screen
* **Persistent Web Configuration**:
  * 9 NVS configuration managers built on `BaseConfigManager`
  * Standalone SoftAP setup portal (`DASSHOME-Setup`) and station mode mDNS (`dasshome.local`)
  * Gzip-compressed embedded Web UI (~77% flash savings)
  * REST API for live JSON configuration inspection, partial saves, and factory resets

The transmitter is being developed alongside the receiver with low-power operation and deep-sleep operation as a major design goal.

---

# Repository Structure

```text
DASS HOME/
│
├── transmitter/
│   └── Transmitter firmware (ESP32-C3 Supermini)
│
├── receiver/
│   └── Receiver firmware (ESP32)
│
├── docs/
│   ├── architecture.md
│   ├── protocol.md
│   ├── hardware.md
│   └── development.md
│
└── README.md
```

The transmitter and receiver are intentionally kept in the same repository because they are two parts of the same system and share the DASS HOME application protocol.

---

# Firmware Architecture

Both firmware projects are organized into modular components.

## Receiver Structure

```text
receiver/
│
├── receiver.ino                         # Main setup() and loop() coordinator
│
├── config/                              # Persistent Configuration & Hardware Pins
│   ├── BaseConfigManager.h              # CRTP base class for NVS persistence & validation
│   ├── BoardConfig.h                    # GPIO pin definitions & hardware constants
│   ├── BatteryLedConfig.h / .cpp        # Battery thresholds, INA219 & 5-LED configuration
│   ├── ConfigJsonHelper.h / .cpp        # REST API JSON serialization & bulk resets
│   ├── DevConfig.h / .cpp               # Developer mode entry toggle
│   ├── DisplayConfig.h                  # Screen sections & page definitions
│   ├── DisplayLayoutConfig.h / .cpp     # Section/page layout & visibility configuration
│   ├── LoRaConfig.h / .cpp              # LoRa frequency, network ID, address, SF, BW
│   ├── PacketConfig.h                   # DASS HOME packet validation rules
│   ├── SystemConfig.h / .cpp            # UI timeouts, long press duration, auto-sleep
│   ├── TankConfig.h / .cpp              # Tank geometry, capacity & distance calibration
│   ├── TimeConfig.h / .cpp              # NTP servers & GMT timezone offset
│   ├── TransmitterConfig.h / .cpp       # Transmitter-to-tank mapping & sensor limits
│   └── WiFiConfig.h / .cpp              # Wi-Fi credentials & connection timeouts
│
├── models/                              # Data Structures
│   ├── DisplayData.h                    # Normalized telemetry data consumed by UI
│   └── Telemetry.h                      # Parsed packet models
│
├── protocol/                            # Protocol Decoding
│   ├── PacketParser.h / .cpp            # DASS HOME key-value payload parser
│
├── lora/                                # LoRa Transport
│   ├── LoRaManager.h / .cpp             # RYLR998 AT command parser & packet receiver
│
├── tank/                                # Domain Logic
│   ├── TankProcessor.h / .cpp           # Calibration math (percentage, volume, battery)
│
├── display/                             # OLED User Interface
│   ├── DisplayManager.h / .cpp          # SH1106 U8g2 driver, screens, animations & sleep
│
├── battery/                             # Fuel Gauge & Power Monitor
│   ├── BatteryLedManager.h / .cpp       # 5-LED fuel gauge controller & charging state
│   └── INA219Driver.h                   # Hardware INA219 I2C current/voltage driver
│
├── input/                               # User Input
│   ├── ButtonManager.h / .cpp           # Debouncing, short-press, long-press, selection
│
├── time/                                # Network Time
│   ├── TimeManager.h / .cpp             # NTP sync & relative elapsed time formatting
│
├── storage/                             # Data Persistence
│   ├── StorageManager.h / .cpp          # Telemetry state caching in ESP32 NVS
│
├── mock/                                # Testing
│   ├── MockDataManager.h / .cpp        # Simulated telemetry generator for offline testing
│
└── wifi/                                # Connectivity & Web Portal
    ├── WiFiManager.h / .cpp             # SoftAP, Station connection, DNS & WebServer
    ├── web_portal.html                  # Editable source HTML/CSS/JS for Config Portal
    ├── generate_portal_gz.py            # Script to compress web_portal.html into C++ header
    └── WebPortalHtml.h                  # Gzipped PROGMEM payload served by WebServer
```

### Module Responsibilities

| Responsibility | Component Files |
|---|---|
| **GPIO & Pinouts** | `config/BoardConfig.h` |
| **NVS Persistent Settings** | `config/BaseConfigManager.h`, `config/*Config.h` |
| **Portal REST API & Serialization** | `config/ConfigJsonHelper.cpp` |
| **Web Portal HTML & Compression** | `wifi/web_portal.html`, `wifi/generate_portal_gz.py`, `wifi/WebPortalHtml.h` |
| **Wi-Fi, AP & Captive Portal** | `wifi/WiFiManager.cpp` |
| **LoRa UART & AT Handling** | `lora/LoRaManager.cpp` |
| **Telemetry Parsing** | `protocol/PacketParser.cpp` |
| **Tank & Battery Calculations** | `tank/TankProcessor.cpp` |
| **OLED Screens & Navigation** | `display/DisplayManager.cpp` |
| **INA219 & 5-LED Fuel Gauge** | `battery/BatteryLedManager.cpp`, `battery/INA219Driver.h` |
| **Button Clicks & Gestures** | `input/ButtonManager.cpp` |
| **NTP & Elapsed Time** | `time/TimeManager.cpp` |
| **Telemetry NVS Cache** | `storage/StorageManager.cpp` |
| **Mock Telemetry Layer** | `mock/MockDataManager.cpp` |

---

# Communication

DASS HOME uses a two-layer packet structure.

The RYLR998 provides the radio transport envelope:

```text
+RCV=<address>,<length>,<data>,<RSSI>,<SNR>
```

Inside the LoRa payload is the DASS HOME application packet:

```text
TS|v=1|id=1|seq=123|dist=43.2|bat=3.87|chg=0
```

Where:

| Field  | Meaning                 |
| ------ | ----------------------- |
| `TS`   | DASS HOME packet header |
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

The receiver operates on standard 3.3V ESP32 hardware:

| Component | Interface / Pins | Details |
|---|---|---|
| **MCU** | ESP32-WROOM-32 | Dual-core 240 MHz, 4 MB Flash |
| **OLED Display** | I2C (SDA: `21`, SCL: `22`) | 1.3" SH1106 128×64 monochrome OLED |
| **Current / Voltage Sensor** | I2C (SDA: `21`, SCL: `22`) | INA219 High-Side DC Monitor (I2C addr `0x40`) |
| **Battery Fuel Gauge** | GPIO `13`, `14`, `25`, `26`, `32` | 5-LED active-HIGH indicators with EMA filter |
| **LoRa Transceiver** | UART2 (RX: `16`, TX: `17`) | Reyax RYLR998 (115200 baud default) |
| **User Button** | GPIO `27` | Active-LOW with internal pullup |

Pin assignments and timeouts can be reviewed in:

```text
receiver/config/BoardConfig.h
receiver/config/BatteryLedConfig.h
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

The firmware is built using the standard Arduino ecosystem for ESP32.

Required libraries for the receiver:

* **ESP32 Arduino Core** (v2.x or v3.x)
* **U8g2** (by Oliver Kraus — installable via Arduino Library Manager)
* **Wire** (Built-in)
* **WiFi**, **WebServer**, **DNSServer**, **Preferences**, **ESPmDNS** (Built-in to ESP32 Core)

---

# Building

## Receiver

1. Open `receiver/receiver.ino` in the Arduino IDE.
2. Select **ESP32 Dev Module** (or your specific ESP32 board).
3. Ensure the **U8g2** library is installed in your Arduino IDE libraries.
4. Compile and upload to your ESP32 board.

## Transmitter

1. Open `transmitter/transmitter.ino`.
2. Select your transmitter board (e.g., **ESP32C3 Dev Module**).
3. Compile and upload.

---

# Persistent Configuration & Web Portal

The receiver features 9 persistent configuration managers backed by ESP32 non-volatile storage (NVS) using a type-safe `BaseConfigManager` architecture.

### Configuration Modules

1. **System Config** (`cfg_system`): UI sleep timeout, config mode timeout, long-press duration, charging animation splash time.
2. **Wi-Fi Config** (`cfg_wifi`): Home Wi-Fi credentials, connection timeout, legacy migration support.
3. **Tank Config** (`cfg_tank`): Up to 4 tanks; total height, total capacity, FULL calibration distance, EMPTY calibration distance.
4. **Transmitter Config** (`cfg_tx`): Up to 4 transmitters; LoRa address mapping, tank association, sensor min/max range, battery voltage range.
5. **Battery & LED Config** (`cfg_battery`): INA219 current thresholds (charging vs. discharging), voltage-to-percent curve table, 5-LED thresholds.
6. **LoRa Config** (`cfg_lora`): RF carrier frequency (default 867 MHz), network ID, node address, spreading factor, bandwidth, coding rate.
7. **Time Config** (`cfg_time`): Primary and secondary NTP servers, GMT timezone offset in seconds, daylight savings offset.
8. **Dev Config** (`cfg_dev`): Toggle to enable/disable the INA219 real-time diagnostic screen.
9. **Display Layout Config** (`cfg_dlayout`): Enable/disable individual display sections and pages.

### Web Configuration Portal

When in Config Mode (triggered on first boot or by double-clicking/long-pressing the button):
- **Access via SoftAP**: Connect to Wi-Fi network `DASSHOME-Setup` (no password), open your browser to `http://192.168.4.1` (captive portal redirects automatically).
- **Access via Local Network**: When connected to home Wi-Fi, open `http://dasshome.local` in any browser on the same network.
- **Gzip Asset Delivery**: The portal HTML/CSS/JS is pre-compressed with gzip (~10.5 KB transfer vs ~47 KB uncompressed), minimizing flash usage and loading in under a second over SoftAP.
- **Editing the Web Portal**:
  1. Edit the clean source HTML at `receiver/wifi/web_portal.html`.
  2. Run `python3 receiver/wifi/generate_portal_gz.py` to regenerate `receiver/wifi/WebPortalHtml.h`.

---

# Power Management

Low-power operation is an important part of the DASS HOME design.

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

DASS HOME follows several design principles.

### 1. Separate configuration from logic

Physical configuration should not be scattered throughout the firmware.

### 2. Separate transport from application protocol

RYLR998 communication and DASS HOME packet parsing are treated as different layers.

### 3. Keep hardware-specific code isolated

OLED, button, LoRa and sensor handling should not leak into unrelated modules.

### 4. Keep calculations independent from the UI

Tank and battery calculations should produce data models that the display can consume.

### 5. Prefer simple embedded-friendly architecture

The project should remain understandable and lightweight enough for ESP32-class hardware.

### 6. Avoid unnecessary dependencies

The core monitoring functionality is self-contained.

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

Change Wi-Fi configuration
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

This repository represents the development version of DASS HOME.

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
