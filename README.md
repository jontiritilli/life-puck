# LifePuck Project

LifePuck is a digital life counter and utility for tabletop games, designed for the [Waveshare ESP32-S3-Touch-LCD-1.85](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85) board (or compatible custom hardware). It features a 1.85" IPS LCD, capacitive touch, and I2C IO expander, built with PlatformIO and LVGL.

## Hardware Reference

- [Waveshare ESP32-S3-Touch-LCD-1.85 Wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85)
  - [Hardware Overview](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85#Hardware_Overview)
  - [Pinout](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85#Pinout)
  - [Schematic](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85#Schematic)
  - [Resources](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85#Resources)

See the wiki for hardware specs, pin mapping, and more.

## Hardware Wiring

- See [ESP32-S3-Touch-LCD-1.85.pdf](ESP32-S3-Touch-LCD-1.85.pdf) for wiring and hardware details.

## Hardware Features

- 1.85" 360x360 IPS LCD (Waveshare ESP32-S3-Touch-LCD-1.85)
- Capacitive touch panel (CST816S)
- I2C IO expander support

## Software Features

- One and two player life counter modes
- Configurable base life settings and quick reset
- In-app configuration menus for settings and preferences
- Change history tracking and undo support
- Grouped change display for clarity
- Smooth LVGL-based UI animations
- Robust state management and crash prevention
- Safe animation and event handling (no boot loops)
- Modular code structure for easy extension
- PlatformIO build system for easy development and deployment

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

---

For more details, see code comments, configuration files, and the [Waveshare ESP32-S3-Touch-LCD-1.85 Wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85).
