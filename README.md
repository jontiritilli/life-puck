# LifePuck Project

This project is for a custom ESP32-S3 board with LCD, touch panel, and IO expander, using PlatformIO and ESP Panel.

## Features
- ESP32-S3 based custom hardware
- LCD display with LVGL graphics
- Capacitive touch panel (CST816S)
- I2C IO expander support
- PlatformIO build system

## Getting Started
1. **Install PlatformIO**: https://platformio.org/install
2. **Connect your ESP32-S3 board** via USB.
3. **Build and upload**:
   ```
   pio run -t upload
   ```
4. **Open the serial monitor**:
   ```
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

---
For more details, see the code comments and configuration files.
