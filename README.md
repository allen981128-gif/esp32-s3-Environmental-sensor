# ESP32-S3 Sensor Node (BME680 + VL53L4CX ToF + GPS) — OLED • SD (CSV) • Web Dashboard

## About
This repository contains the firmware for an ESP32-S3 sensor node built with **PlatformIO** (Arduino framework). The device reads environmental data (BME680), distance (VL53L4CX ToF), and GPS (NMEA), displays key outputs on an SSD1306 OLED, optionally logs measurements to a microSD card in CSV format, and provides a local web dashboard over Wi-Fi SoftAP.

---

## Features
- **BME680**: temperature, humidity, pressure, gas indicator (mapped to a relative AQI-style value and smoothed in firmware)
- **VL53L4CX ToF**: distance measurement with validity checks (`-1` when invalid/no target)
- **GPS**: continuous NMEA parsing in the main loop; returns latest parsed coordinates/time
- **OLED**: local display of key sensor outputs and system/logging status
- **SD logging**: CSV append logging, 50 Hz (20 ms), auto-stops after 100 samples
- **Web dashboard (SoftAP)**: live status JSON + controls + CSV download

---

## Hardware
- **Board**: Adafruit Feather ESP32-S3
- **Sensors**: BME680 (I2C), VL53L4CX ToF (I2C + XSHUT), GPS module (UART)
- **Display**: SSD1306 OLED (128×64, I2C)
- **Storage**: microSD (SPI)

Pin-level details (GPS RX/TX, ToF XSHUT, SD CS) are defined in the firmware source. If you have a wiring diagram or photos, place them in `docs/figures/` and reference them here.

---

## Wi-Fi Dashboard
The device runs as a Wi-Fi Access Point (SoftAP).

- **SSID**: `Sensor-AP`
- **Password**: `12345678`
- **Dashboard**: `http://192.168.4.1/`  (open this in a browser)

### API Endpoints
- `GET  /api/status` — current system status (JSON)
- `POST /api/system/start` — enter measurement mode (equivalent to BOOT press)
- `POST /api/log/start` — start SD logging
- `POST /api/log/stop` — stop SD logging
- `POST /api/log/clear` — clear and recreate CSV log
- `GET  /download` (alias: `/data`) — download `log.csv`

---

## Controls (BOOT Button)
- **Short press**: toggle logging (start/stop)
- **Long press (> 2 s)**: clear the SD log file

---

## SD Logging (CSV)
- File: `/log.csv`
- Interval: **20 ms** (50 Hz)
- Auto-stop: **100 samples**
- If `/log.csv` does not exist, the firmware creates it and writes a header row.
- Notes:
  - ToF invalid/no target → `distance = -1` (OLED shows `-- mm`)
  - GPS no fix → latitude/longitude and timestamp remain `--`

---

## Repository Structure
```text
.
├── src/                 # main.cpp + module .cpp files
├── include/             # module headers (.h)
├── lib/                 # local libraries (if any)
├── docs/                # report and figures/screenshots
│   ├── report.pdf
│   └── figures/
├── platformio.ini       # PlatformIO build configuration
├── .gitignore
└── README.md
```

---

## Build & Flash (PlatformIO)

### Environment (platformio.ini)
This project uses the following PlatformIO environment:

- `[env:adafruit_feather_esp32s3]`
- `platform = espressif32`
- `board = adafruit_feather_esp32s3`
- `framework = arduino`
- `monitor_speed = 115200`
- `upload_speed = 921600`
- `build_flags`:
  - `-DARDUINO_USB_MODE=0`
  - `-DARDUINO_USB_CDC_ON_BOOT=1`

### Dependencies (platformio.ini)
Declared via `lib_deps`:
- `WiFi`
- `adafruit/Adafruit Unified Sensor@^1.1.14`
- `adafruit/Adafruit BME680 Library@^2.0.2`
- `adafruit/Adafruit GPS Library@^1.7.2`
- `adafruit/Adafruit SSD1306@^2.5.9`
- `adafruit/Adafruit GFX Library@^1.11.10`
- `adafruit/Adafruit BusIO@^1.15.0`
- `adafruit/Adafruit SPIFlash@^4.2.3`
- `adafruit/Adafruit NeoPixel@^1.12.3`

### Build (CLI)
```bash
pio run -e adafruit_feather_esp32s3
```

### Upload (CLI)
```bash
pio run -e adafruit_feather_esp32s3 -t upload
```

### Serial Monitor (CLI)
```bash
pio device monitor -b 115200
```

---

## Usage
1. Flash the firmware to the ESP32-S3.
2. Power the board and connect your phone/laptop to Wi-Fi `Sensor-AP` (password `12345678`).
3. Open `http://192.168.4.1/`.
4. Press **BOOT** (or click “Start System” on the dashboard) to enter run mode.
5. Start/stop logging using BOOT short press or the dashboard controls.
6. Download the CSV using `/download` from the dashboard.

---

## Troubleshooting
- **Upload fails / no serial output**: confirm the correct device/port, then retry with the configured baud rate `115200`.
- **SD not detected**: ensure the SD card is formatted (FAT32 recommended) and wiring matches the firmware CS pin definition; check OLED/dashboard SD status.
- **No GPS fix**: GPS may require a clear view of the sky; without fix, coordinates/time remain `--`.
- **ToF shows `-- mm`**: no valid target or invalid range status; firmware returns `-1` in this case.

---

## Report
- Final report: `docs/report.pdf`
- Figures/screenshots: `docs/figures/`

---

## License
MIT License (see `LICENSE`).
