| Supported Targets | ESP32 | ESP32-S3 |
| ----------------- | ----- | -------- |

# Life Puck Project

Life Puck is a custom ESP32-S3-based project featuring a touchscreen LCD, battery management, sensors, and a modern UI built with LVGL. This project is designed for embedded development using the ESP-IDF framework.

## Features

- ESP32-S3 microcontroller
- 1.85" Touch LCD display
- LVGL graphics library
- Battery monitoring and power management
- I2C, SD card, RTC, and sensor drivers
- Modular C++ codebase

## Project Structure

```
├── CMakeLists.txt
├── sdkconfig
├── main/
│   ├── arc_animation.cpp/.h
│   ├── boot_screen.cpp/.h
│   ├── gestures.cpp/.h
│   ├── life_counter.cpp/.h
│   ├── LifePuck.cpp
│   ├── lv_conf.h
│   ├── lvgl_config.cpp
│   ├── menu.cpp/.h
│   ├── BAT_Driver/
│   ├── EXIO/
│   ├── I2C_Driver/
│   ├── LCD_Driver/
│   ├── LVGL_Driver/
│   ├── PCF85063/
│   ├── PWR_Key/
│   ├── QMI8658/
│   ├── SD_Card/
│   └── Touch_Driver/
├── managed_components/
│   └── ...
└── README.md
```

## Getting Started

1. **Install ESP-IDF**: Follow the [ESP-IDF Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/index.html) for your platform.
2. **Set up submodules/components**: Ensure all dependencies in `managed_components/` are present.
3. **Configure the project**:
   ```sh
   idf.py menuconfig
   ```
4. **Build the project**:
   ```sh
   idf.py build
   ```
5. **Flash to device**:
   ```sh
   idf.py -p <PORT> flash
   ```
6. **Monitor output**:
   ```sh
   idf.py -p <PORT> monitor
   ```

## Troubleshooting

- **Program upload failure**
  - Check hardware connections and USB cable.
  - Lower the baud rate in `menuconfig` if flashing fails.
  - Use `idf.py -p <PORT> monitor` and reboot to check logs.

## Technical Support and Feedback

- For technical queries, visit the [esp32.com](https://esp32.com/) forum
- For feature requests or bug reports, create a [GitHub issue](https://github.com/espressif/esp-idf/issues)

---

This project is not affiliated with Espressif. For ESP-IDF documentation, see the [official guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/).
