#pragma once

// *** AUTO-DETECT VIA PLATFORMIO ***
// Kein manuelles Kommentieren mehr!

#if defined(BOARD_1_85C)
    #define HAS_GPIO_EXTENDER  1
    
    // TCA9554 (GPIO Extender)
    #define EXIO_PIN_SDA  8
    #define EXIO_PIN_SCL  17
    #define EXIO_PIN_INT  16
    
    // Reset Pins via EXIO
    #define LCD_PIN_RST   -1
    #define TOUCH_PIN_RST -1
    
    #define BOARD_NAME "ESP32-S3-Touch-LCD-1.85C"
    
#elif defined(BOARD_1_85)
    #define HAS_GPIO_EXTENDER  1  // ← Demo zeigt: nicht-C hat AUCH GPIO-Expander!
    
    // TCA9554 (GPIO Extender) - wie bei C-Variante
    #define EXIO_PIN_SDA  8
    #define EXIO_PIN_SCL  17
    #define EXIO_PIN_INT  16
    
    // Reset Pins via EXIO (wie Demo zeigt)
    #define LCD_PIN_RST   14  // Kann trotzdem direkt sein
    #define TOUCH_PIN_RST -1  // ← Demo: Touch Reset über EXIO!
    
    #define BOARD_NAME "ESP32-S3-Touch-LCD-1.85"
    
#else
    #error "Kein Board definiert! Wähle env:board_1_85C oder env:board_1_85 in PlatformIO!"
#endif

// ========================================
// GEMEINSAME DEFINES (BEIDE GLEICH!)
// ========================================
#define LCD_WIDTH  360
#define LCD_HEIGHT 360

// LCD Pins
#define LCD_PIN_CS     21
#define LCD_PIN_SCK    40
#define LCD_PIN_DATA0  46
#define LCD_PIN_DATA1  45
#define LCD_PIN_DATA2  42
#define LCD_PIN_DATA3  41
#define LCD_PIN_TE     18
#define LCD_PIN_BL     5

// I2C Pins (gemeinsam für Hauptsystem)
#define I2C_SDA_PIN    11
#define I2C_SCL_PIN    10

// Touch Pins (unterschiedlich je Board!)
#if defined(BOARD_1_85C)
    // C-Variante: Touch über Haupt-I2C
    #define TOUCH_PIN_SDA  I2C_SDA_PIN  // 11
    #define TOUCH_PIN_SCL  I2C_SCL_PIN  // 10
    #define TOUCH_PIN_INT  4
#else
    // Nicht-C: Touch hat separaten I2C-Bus (Demo-Werte!)
    #define TOUCH_PIN_SDA  1   // ← Demo: CST816_SDA_PIN = 1
    #define TOUCH_PIN_SCL  3   // ← Demo: CST816_SCL_PIN = 3
    #define TOUCH_PIN_INT  4
#endif
