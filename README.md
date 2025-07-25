# LifePuck Project

LifePuck is a digital life counter and utility for tabletop and TCG games, designed for the [Waveshare ESP32-S3 1.85inch Round Display](https://www.waveshare.com/esp32-s3-touch-lcd-1.85.htm) board. It features a 1.85" LCD screen with capacitive touch.

The goal of this project is to provide a simple, intuitive, and visually appealing life counter for Flesh and Blood, Magic: The Gathering, Pokémon TCG, and others. It supports both single-player and two-player modes, with customizable settings, additional modes quick reset options, and a robust history.

If you have a capable 3D printer, you can also print a case for the device. The case is designed to be compact and portable, making it easy to carry around. It also fits the Waveshare ESP32-S3 board and the battery snugly, providing protection and a professional look.

## Product Links

- [Waveshare Board Amazon link](https://a.co/d/g4B9fnk)
- [Battery Amazon Link](https://a.co/d/2o6Rkg9)
- [3d Printed Case](https://makerworld.com/en/models/1635526-game-puck#profileId-1727766)

## Hardware Information

- [Waveshare ESP32-S3-Touch-LCD-1.85 Wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85)
  - [Hardware Overview](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85#Hardware_Overview)
  - [Pinout](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85#Pinout)
  - [Schematic](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85#Schematic)
  - [Resources](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85#Resources)

## 3D Printed Case

The case was designed by someone else. I remixed it and modified it (within the license parameters) to be more fitting for this use case. You can find the design files here: [Game Puck Case](https://makerworld.com/en/models/1635526-game-puck#profileId-1727766)

## Software Features

- One and two player life counter modes
- Configurable base life settings
- Quick reset
- In-app configuration menus for settings and preferences
- Change history tracking
- Event grouping for precise control over adjustments to life totals
- Smooth LVGL-based UI animations
- Robust state management and crash prevention
- Modular code structure for easy extension

## Getting Started

1. **Clone this repository**
1. **Install PlatformIO:** [https://platformio.org/install](https://platformio.org/install)
1. **Connect your ESP32-S3 board** via USB.
1. **Build and upload:**

   ```sh
   pio run -t upload
   ```

1. **Open the serial monitor:**

   ```sh
   pio device monitor -b 115200
   ```

---

For more details, see code comments, configuration files, and the [Waveshare ESP32-S3-Touch-LCD-1.85 Wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85).
