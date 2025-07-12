# LifePuck Project

This project targets the [Waveshare ESP32-S3-Touch-LCD-1.85](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85) board (or compatible custom hardware) with LCD, capacitive touch, and IO expander, using PlatformIO and ESP Panel.

## Board Wiki

- **Waveshare ESP32-S3-Touch-LCD-1.85 Wiki:** [https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85)
  - [Hardware Overview](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85#Hardware_Overview)
  - [Pinout](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85#Pinout)
  - [Schematic](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85#Schematic)
  - [Driver and Example Code](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85#Resources)

Refer to the wiki for detailed hardware specs, pin mapping, and additional resources.

## Hardware Wiring

- See the included [ESP32-S3-Touch-LCD-1.85.pdf](ESP32-S3-Touch-LCD-1.85.pdf) for the full wiring diagram and hardware details for this project.

## Features

- ESP32-S3 based custom hardware (Waveshare ESP32-S3-Touch-LCD-1.85)
- 1.85" 240x280 IPS LCD display with LVGL graphics
- Capacitive touch panel (CST816S)
- I2C IO expander support
- PlatformIO build system

## Getting Started

1. **Install PlatformIO:** [https://platformio.org/install](https://platformio.org/install)

2. **Connect your ESP32-S3 board** via USB.

3. **Build and upload:**

   ```sh
   pio run -t upload
   ```

4. **Open the serial monitor:**

   ```sh
   pio device monitor -b 115200
   ```

## File Structure

- `src/main.cpp` - Main application code
- `src/esp_panel_board_custom_conf.h` - Custom board configuration
- `src/lv_conf.h` - LVGL configuration
- `src/i2c_scanner.cpp` - I2C scanner utility (for hardware debug)
- `platformio.ini` - PlatformIO project configuration

## Troubleshooting

- If you see a static or garbled screen before LVGL loads, this is normal. LVGL will take over after initialization.
- For I2C device detection, use the I2C scanner code in `src/i2c_scanner.cpp`.
- Make sure the correct serial port and baud rate (115200) are selected in your monitor.

## Credits

- Based on Espressif ESP Panel and LVGL
- Touch driver: CST816S
- Hardware: [Waveshare ESP32-S3-Touch-LCD-1.85](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85)

---

For more details, see the code comments, configuration files, and the [Waveshare ESP32-S3-Touch-LCD-1.85 Wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85).
