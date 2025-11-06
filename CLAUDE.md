# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Life Puck** is an embedded firmware project for ESP32-S3 devices that provides a digital life counter for Magic: The Gathering, Yu-Gi-Oh!, Pokémon TCG, and other tabletop card games. It runs on Waveshare ESP32-S3 round display modules (1.85" LCD, 360x360px) with capacitive touch screens.

### Key Technologies
- **Platform:** ESP32-S3 (16MB Flash, 8MB PSRAM)
- **Framework:** Arduino 3.x (ESP-IDF 5.3 via pioarduino)
- **Display:** ST77916 driver (QSPI mode), 1.85" round LCD
- **Touch:** CST816 capacitive touch controller
- **GUI:** LVGL 9.3.0
- **Storage:** NVS (Non-Volatile Storage) for persistent settings

## Build Commands

### Build the project
```bash
pio run
```

### Upload to device
```bash
pio run -t upload
```

### Build and upload in one step
```bash
pio run -t upload && pio device monitor
```

### Serial monitor (115200 baud)
```bash
pio device monitor
```

### Clean build
```bash
pio run -t clean && pio run
```

### Erase flash completely
```bash
pio run -t erase
```

### Switch between board variants
Edit `platformio.ini` and change `default_envs`:
- `default_envs = board_1_85C` (ESP32-S3-Touch-LCD-1.85C with GPIO expander)
- `default_envs = board_1_85` (ESP32-S3-Touch-LCD-1.85)

## Architecture Overview

### Layered Architecture

The codebase follows a **hardware abstraction layer (HAL)** pattern with clear separation of concerns:

```
┌─────────────────────────────────────┐
│   UI Layer (LVGL screens/widgets)   │
├─────────────────────────────────────┤
│   Application Layer (game logic)    │
├─────────────────────────────────────┤
│   Hardware Layer (drivers/HAL)      │
├─────────────────────────────────────┤
│   Config Layer (board/user config)  │
└─────────────────────────────────────┘
```

### Directory Structure

#### `src/config.h` - Single Source of Truth
**CRITICAL:** All user-configurable settings (theme colors, animation timing, presets, touch calibration defaults, display brightness, audio settings) are defined here. Always check this file first when working with configurable values.

#### `include/board_config.h` - Hardware Configuration
Board-specific pin definitions and hardware features. Auto-detects board variant via PlatformIO build flags (`BOARD_1_85C` or `BOARD_1_85`). Defines pins for LCD, touch, I2C, GPIO expander.

#### `src/core/` - Application Core
- **`main.cpp`**: Arduino setup()/loop(), system initialization sequence
- **`state_manager.cpp/h`**: NVS wrapper for persistent storage (settings, presets, game state)
- **`gui_main.cpp/h`**: LVGL initialization and main UI entry point

#### `src/hardware/` - Hardware Abstraction Layer
- **`display/`**: ST77916 LCD driver, LVGL integration, display buffer management
- **`touch/`**: CST816 touch controller driver, gesture detection
- **`peripherals/`**: I2C driver, TCA9554 GPIO expander, power button handling
- **`system/`**: Battery monitoring, power management, sleep/wake logic
- **`audio/`**: Simple buzzer/audio feedback system

#### `src/ui/` - User Interface
- **`screens/life/`**: Main life counter screens (1P and 2P modes), simple counters (poison/commander damage/etc.)
- **`screens/menu/`**: Tools menu, preset editor, dice/coin screens
- **`screens/settings/`**: Settings overlays (brightness, audio, timer, theme, touch calibration)
- **`screens/tools/`**: Timer, dice roller, coin flip
- **`components/`**: Reusable UI components
- **`helpers/`**: Gesture detection, animation helpers, tap layer, event grouping

#### `src/data/` - Data Management
- **`tcg_presets.cpp/h`**: TCG game presets (MTG, Yu-Gi-Oh!, Pokemon, etc.) - loads from NVS or config.h
- **`themes.cpp/h`**: Theme system (background images, colors, opacity)
- **`history.cpp/h`**: Game history tracking (life changes)
- **`constants.h`**: NVS keys and shared constants

### Critical Initialization Order

**Order matters!** From `src/main.cpp` setup():

1. **I2C_Init()** - Initialize I2C bus
2. **TCA9554PWR_Init()** - Initialize GPIO expander (if present)
3. **power_init()** - MUST be called early! Sets up power button, prevents instant shutdown on non-C model
4. **LCD_Init()** + **Backlight_Init()** - Initialize display
5. **Lvgl_Init()** - Initialize LVGL GUI system
6. **battery_init()** - Initialize battery monitoring
7. **simple_audio_init()** - Initialize audio system
8. **init_presets()** - Load TCG presets from NVS (initializes NVS)
9. **power_management_init()** - Initialize power management (after NVS)
10. **ui_init()** - Create UI screens
11. **Touch calibration check** - Show calibration dialog if needed

### State Management & Persistence

**NVS (Non-Volatile Storage)** is used extensively for all persistent data:
- Managed via `StateStore` class in `src/core/state_manager.cpp/h`
- Global instance: `player_store` (namespace: "config")
- All NVS keys defined in `src/data/constants.h` (prefixed with `KEY_`)
- Settings, presets, life totals, calibration data, and theme selections survive power cycles

**EventGrouper System** (`src/ui/helpers/event_grouper.cpp/h`):
- Batches rapid life changes before saving to NVS
- Reduces wear on flash memory
- Window: 1500ms (configurable via `GROUPER_WINDOW` in config.h)

### Touch Calibration System

**Advanced 7-parameter affine transformation matrix** (`src/hardware/display/lvgl_driver.cpp`):
- Defaults defined in `src/config.h` (lines 245-251)
- Parameters: `OFFSET_X`, `OFFSET_Y`, `SCALE_X`, `SCALE_Y`, `SHEAR_XY`, `SHEAR_YX`, `DIVISOR`
- User can calibrate via on-device 3-round calibration screen
- Calibration data saved to NVS and loaded on boot
- **Debugging tip:** Use preset editor keyboard to test calibration accuracy

### Board Variant Handling

Two hardware variants with subtle differences:
- **1.85C**: Has TCA9554 GPIO expander, touch on main I2C bus (pins 10/11)
- **1.85**: Also has TCA9554, touch on separate I2C bus (pins 1/3)

Board detection is automatic via PlatformIO build flags in `platformio.ini`. Code uses `#if defined(BOARD_1_85C)` for conditional compilation.

### Power Management

**Critical timing issue:** On non-C model (button-based power), the device will shut down immediately after button release unless `power_init()` is called early in setup(). The C model (switch-based) doesn't have this issue.

Power states managed by `src/hardware/system/power_management.cpp`:
- Auto-dim after inactivity (configurable)
- Auto-sleep after extended inactivity
- Low battery dimming
- Wake-up via touch interrupt

## Important Development Guidelines

### Modifying User-Configurable Settings
1. **Always edit `src/config.h` first** - this is the single source of truth
2. Rebuild and upload firmware to apply changes
3. For force-reset flags (`FORCE_FACTORY_RESET`, etc.), set back to `false` after testing

### Working with TCG Presets
- Preset defaults: `src/config.h` (lines 106-173)
- Runtime loading/saving: `src/data/tcg_presets.cpp`
- Users can edit presets on-device via preset editor
- To force defaults, set `FORCE_DEFAULT_PRESETS true` in config.h

### LVGL Version Compatibility
Project uses **LVGL 9.3.0** (v9 API). Key differences from LVGL 8:
- Display driver: `lv_display_t` (not `lv_disp_drv_t`)
- Input device: `lv_indev_t` (not `lv_indev_drv_t`)
- Function signatures changed (e.g., flush callback)

### Memory Management
- **PSRAM available:** 8MB external PSRAM for large allocations
- **LVGL buffer:** 80 lines * 360px (defined in `LVGL_Driver.h`)
- Large allocations (≥4KB) automatically use PSRAM via `heap_caps_malloc_extmem_enable()`

### Touch & Gesture Detection
- Raw touch data: `src/hardware/touch/Touch_CST816.cpp`
- Gesture processing: `src/ui/helpers/gestures.cpp`
- Tap layer system: `src/ui/helpers/tap_layer.cpp`
- Touch coordinates transformed via calibration matrix in LVGL driver

**Gesture Control Mode** (runtime configurable via Settings menu or `config.h`):

**Mode 0: CLASSIC MODE (Original Behavior)**
- **Tap top/bottom**: Small life adjustment (±1)
- **Swipe up/down**: Large life adjustment (±5) - no repeat
- **Long press center**: Opens settings menu

**Mode 1: HOLD MODE (New Behavior - Default)**
- **Tap top/bottom**: Small life adjustment (±1)
- **Long press top/bottom**: Large life adjustment (±5), **repeats continuously while held**
- **Swipe down**: Opens settings menu

**Applies to both 1-Player and 2-Player modes** with appropriate zone detection.

**Configuration:**
- **Runtime**: Toggle between "Classic" and "Hold" modes in Settings overlay (button in Row 6, Col 1)
- **Compile-time default**: `GESTURE_CONTROL_MODE` in `config.h` (0=Classic, 1=Hold)
- **Storage**: Saved to NVS as `KEY_GESTURE_MODE`, persists across reboots

**Timing Configuration** (in `config.h`):
- `LONG_PRESS_TIME_MS`: Initial delay before long press triggers (default: 500ms)
- `LONG_PRESS_REPEAT_TIME_MS`: Repeat interval while holding in Hold Mode (default: 500ms = 2 steps/second)

**Important Note:** The "Dismiss" button (Row 6, Col 0) controls swipe-to-dismiss for overlays/menus, which is separate from the gesture control mode.

### Theme System
Themes consist of:
- Background image (optional, from `src/assets/images/`)
- Button color (defined in config.h)
- Text color (for contrast)
- Opacity level (0-255)

Theme application in `src/data/themes.cpp`, triggered by preset selection or manual override.

## Common Development Tasks

### Adding a New TCG Preset
1. Edit `src/config.h` - add preset defines (name, starting_life, steps, theme)
2. Rebuild and upload firmware
3. Preset appears in game selection menu

### Adjusting Touch Calibration Defaults
1. Edit `src/config.h` - modify `TOUCH_CAL_DEFAULT_*` values (lines 245-251)
2. Set `FORCE_DEFAULT_CALIBRATION true` temporarily
3. Upload and test
4. Set `FORCE_DEFAULT_CALIBRATION false` when done

### Modifying Animation Timing
Edit `src/config.h`:
- Dice/coin animations: lines 74-86
- Arc animations: line 348
- Event grouper window: line 349

### Adding a New Settings Screen
1. Create files in `src/ui/screens/settings/`
2. Add screen creation function
3. Call from settings overlay menu in `settings_overlay.cpp`
4. Use `player_store.getInt()/putInt()` for persistence

## Debugging Tips

### Serial Monitor Output
The device outputs detailed debug info at 115200 baud:
- Hardware initialization status
- NVS load/save operations
- Touch events and coordinates
- Power state changes
- Theme loading

### Touch Issues
- If touch is unresponsive: Check I2C scanner output, verify CST816 address (0x15)
- If touch is misaligned: Adjust calibration matrix in config.h
- If touch works but UI doesn't respond: Check if calibration screen is active (disables life counter)

### Display Issues
- Garbage on screen: Wrong board variant selected in platformio.ini
- Inverted colors: Toggle `LCD_CMD_INVON`/`LCD_CMD_INVOFF` in `Display_ST77916.cpp`
- Backlight not working: Check GPIO expander initialization (TCA9554)

### NVS Issues
- Settings not saving: Check serial output for NVS errors
- Corrupted data: Use `FORCE_NVS_CLEAR` flag to erase all stored data
- Presets reverting: Use `FORCE_DEFAULT_PRESETS` to skip NVS loading

### Power Issues
- Device shuts down immediately (non-C model): Ensure `power_init()` is called early in setup()
- Sleep not working: Check auto-dim/sleep timers in settings
- Battery percentage wrong: Battery monitoring uses ADC on GPIO (check `battery_state.cpp`)

## Testing Workflow

1. **Build:** `pio run`
2. **Upload:** `pio run -t upload`
3. **Monitor:** `pio device monitor` (watch for errors)
4. **Test touch:** Use preset editor keyboard (Settings → Edit Presets)
5. **Verify persistence:** Change settings, restart device, verify they persist
6. **Test sleep/wake:** Let device auto-sleep, wake with touch
