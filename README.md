# Life Puck - Multi-Hardware Fork

[![Platform](https://img.shields.io/badge/platform-ESP32--S3-blue.svg)](https://www.espressif.com/en/products/socs/esp32-s3)
[![Framework](https://img.shields.io/badge/framework-Arduino-00979D.svg)](https://www.arduino.cc/)
[![LVGL](https://img.shields.io/badge/LVGL-9.3.0-green.svg)](https://lvgl.io/)
[![License](https://img.shields.io/github/license/jontiritilli/life-puck)](LICENSE)

**A digital life counter for Magic: The Gathering, Yu-Gi-Oh!, Pokémon TCG, and other tabletop games.**

This is a fork of [jontiritilli/life-puck](https://github.com/jontiritilli/life-puck) with extended hardware support for multiple Waveshare ESP32-S3 display variants.

---

## 📋 Table of Contents

- [Hardware Support](#-hardware-support)
- [New Features](#-new-features-in-this-fork)
- [Software Features](#-software-features)
- [Getting Started](#-getting-started)
- [Installation Guide](#-installation-guide)
- [Touch Calibration](#-touch-calibration)
- [Adding Custom Presets](#-adding-custom-presets)
- [Usage Guide](#-usage-guide)
- [Troubleshooting](#-troubleshooting)
- [Contributing](#-contributing)
- [Credits](#-credits)
- [License](#-license)

---

## 🔧 Hardware Support

This fork supports **two** Waveshare ESP32-S3 display variants:

### Supported Boards

| Board | Display | Touch | GPIO Expander | AliExpress Bundle |
|-------|---------|-------|---------------|-------------------|
| **[ESP32-S3-Touch-LCD-1.85C](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85C)** | ST77916 (QSPI) | CST816 | ✅ TCA9554 | ✅ **Recommended** (includes case + battery) |
| **[ESP32-S3-Touch-LCD-1.85](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85)** | ST77916 (QSPI) | CST816 | ❌ None | ❌ More expensive |

> **💡 Cost Tip:** The **1.85C variant** (at least in Europe) is significantly cheaper on AliExpress when purchased as a bundle with case and battery!

### Hardware Specifications

- **MCU:** ESP32-S3-WROOM-1 (N16R8) - 16MB Flash, 8MB PSRAM
- **Display:** 1.85" Round LCD, 360x360px, ST77916 driver (QSPI mode)
- **Touch:** Capacitive touch screen, CST816 controller
- **Connectivity:** USB-C, Wi-Fi, Bluetooth

---

## 🆕 New Features in This Fork

### Hardware Support
- ✅ **Dual board support**: ESP32-S3-Touch-LCD-1.85C and 1.85
- ✅ **GPIO Expander handling**: Automatic detection and configuration for TCA9554
- ✅ **Unified codebase**: Single project for both hardware variants

### Build System Improvements
- ✅ **Arduino 3.x support**: Uses latest ESP-IDF 5.3 based framework via [pioarduino](https://github.com/pioarduino/platform-espressif32)
- ✅ **Multi-environment builds**: Switch between boards with `platformio.ini` environments
- ✅ **Modern LVGL 9.3.0**: Updated graphics library with performance improvements

### New Game Features
- ✅ **🎲 Dice Roller**: Roll D4, D6, D8, D10, D12, and D20
- ✅ **🪙 Coin Flip**: Coin flip for random heads/tails decisions
- ✅ **⚙️ Game Presets**: Pre-configured settings for popular TCGs (MTG, Yu-Gi-Oh!, Pokémon, FaB, Lorcana, One Piece)
- ✅ **✏️ Preset Editor**: Create and save custom game configurations
- ✅ **⏱️ Round Timer**: Track round duration with stopwatch and countdown modes
- ✅ **📊 Enhanced Timer**: Switchable timer modes (stopwatch/countdown) with configurable round times

### User Experience
- ✅ **Comprehensive documentation**: Detailed setup and configuration guides
- ✅ **Preset system**: Pre-compile custom game presets to avoid on-device keyboard


---

## 📱Software Features

### Core Life Tracking
- **One and Two Player Modes**: Track life totals for solo or head-to-head games
- **Configurable Base Life**: Set starting life from 1-999999
- **Increment/Decrement Steps**: Adjust life in small (tap) or large (swipe) steps
- **Life Change History**: Track all life changes

### Game Utilities

#### 🎲 Dice Roller
- Roll standard RPG/TCG dice (D4, D6, D8, D10, D12, D20)
- Clear result display
- Accessible from main menu

#### 🪙 Coin Flip
- 50/50 heads/tails outcomes
- Perfect for tiebreakers and random decisions

#### ⏱️ Timer Modes
- **Stopwatch Mode**: Count up from 00:00 to track game duration
- **Countdown Mode**: Set a round time and count down with visual progress
- **Persistent Timer**: Continues running across screen changes
- **Quick Controls**: Tap to start/pause, long-press to reset

#### ⚙️ Game Presets
Pre-configured settings for popular games:
- **MTG Standard** - 20 life, ±1/±5 steps
- **MTG Commander (EDH)** - 40 life, ±1/±10 steps
- **Pokémon TCG** - 60 life, ±10/±30 steps
- **Yu-Gi-Oh!** - 8000 LP, ±50/±500 steps
- **Flesh and Blood** - 40 life, ±1/±5 steps
- **Lorcana** - 20 lore, ±1/±5 steps
- **One Piece** - 5 life, ±1/±2 steps
- **Custom Slots 8-10** - Fully customizable

#### 📊 Simple Counters System
**New Feature**: 4 configurable auxiliary counters for tracking additional game metrics:

**Counter Features:**
- **4 Corner Positions**: Top-Left, Top-Right, Bottom-Left, Bottom-Right
- **Individual Enable/Disable**: Each counter can be turned on/off independently
- **Touch Interaction**: 
  - **Short Tap**: Increment by 1
  - **Medium Hold**: Decrement by 1
  - **Long Hold**: Reset to 0
- **Visual Feedback**: Color changes during press (yellow for decrement, red for reset)
- **Theme Integration**: Counters adapt to selected theme colors


**Configuration:**
- **Settings → Counters**: Enable/disable individual counters
- **Centralized Settings**: All counter parameters configurable in `config.h`
- **Positioning**: Adjustable X/Y coordinates for each corner
- **Visual Settings**: Size, opacity, colors, and timing thresholds
- **Reset Integration**: Counters reset when main life counter is reset

**Common Uses:**
- **MTG**: Commander damage, poison counters, storm count, energy counters
- **Pokémon**: Energy counters, damage counters, special conditions
- **Yu-Gi-Oh!**: Life Point modifiers, special counters
- **General**: Any auxiliary tracking needed during gameplay


#### ✏️ Preset Editor
- Create custom game configurations
- Set starting life, step size, and theme
- Name your presets (up to 15 characters)
- Save to non-volatile storage
- Quick-select from game menu

#### 🔊 Audio Settings
**Dedicated Audio submenu with:**
- **Audio Toggle**: Enable/disable all sounds
- **Volume Control**: 0-100% (displayed as percentage, 9 internal steps)
- **Sound Selection**: Choose from 4 distinct alert sounds
- Visual feedback with grayed-out disabled states

**Sound Options:**
1. **Sound 1**: Standard beep
2. **Sound 2**: High-pitched tone
3. **Sound 3**: Melodic chime
4. **Sound 4**: Low buzz

### Power Management Features

#### 🔋 Power Settings Submenu
Comprehensive power management in a dedicated submenu:

**Brightness Control:**
- Manual adjustment (0-100%)

**Auto-Dim:**
- Automatically reduces brightness to 25% after inactivity
- Default: 60 seconds
- Configurable timeout (0 = disabled)
- Restores full brightness on touch

**Display Sleep:**
- Turns off display completely after extended inactivity
- Default: 300 seconds (5 minutes)
- Configurable timeout (0 = disabled)
- Wakes on touch

**Low Battery Dimming:**
- Activates automatically at ≤15% battery
- Forces display to 5% brightness
- Prevents dimming when USB charging detected 
- Automatically restores brightness when battery > 15%
- Works independently from auto-dim and sleep

**Critical Battery Protection:**
- **Automatic deep sleep at ≤2% battery**
- **Smart USB detection:**
  - Voltage < 1V → USB charging detected, no shutdown
  - 30-second timeout for switch toggling protection
  - 10-second boot grace period for voltage stabilization
- **Prevents battery damage from deep discharge**
- **Protects against false triggers during charging**


### User Interface
- **LVGL-based UI**: Hardware-accelerated animations
- **Intuitive Touch Controls**: Tap, swipe, and long-press gestures
- **In-app Configuration**: Change all settings without recompiling
- **Visual Feedback**: Animated responses to all interactions
- **Auto-brightness**: Adjustable display brightness to save battery
- **Sleep Mode**: Auto-sleep after configurable inactivity period

### State Management
- **Robust State Persistence**: All settings saved to NVS
- **History Tracking**: Persistent life change log until Reset
- **Undo/Redo Support**: Step back through life changes
- **Session Recovery**: Resume game state after restart

### Swipe Close (Swipe)
- **when activated**: All Lists an Settings menus can be closed by swiping from left to right 

---

## 🚀 Getting Started

### Prerequisites

- **Operating System:** Windows, macOS, or Linux
- **IDE:** Visual Studio Code (recommended)
- **Knowledge:** Basic understanding of C++ and embedded systems
- **Hardware:** One of the supported Waveshare ESP32-S3 boards

### Required Software

1. **Visual Studio Code**: [Download](https://code.visualstudio.com/)
2. **PlatformIO Extension**: Install from VS Code marketplace
3. **USB Drivers**: CH340 (usually auto-installed)

---

## 📦 Installation Guide

### Step 1: Install Visual Studio Code

1. Download VS Code from [https://code.visualstudio.com/](https://code.visualstudio.com/)
2. Install and launch VS Code

### Step 2: Install PlatformIO

1. Open VS Code
2. Go to **Extensions** (Ctrl+Shift+X)
3. Search for **"PlatformIO IDE"**
4. Click **Install**
5. Restart VS Code when prompted

### Step 3: Clone This Repository

**Option A: Using Git (recommended)**

git clone https://github.com/Platinplayer23/Life-Puck---Multi-Hardware-Fork.git
cd Life-Puck---Multi-Hardware-Fork-master


**Option B: Download ZIP**
1. Click the green **"Code"** button on GitHub
2. Select **"Download ZIP"**
3. Extract the ZIP file

### Step 4: Open Project in VS Code

1. Open VS Code
2. File → Open Folder
3. Select the `Life-Puck---Multi-Hardware-Fork-master` folder
4. PlatformIO will automatically detect the project

### Step 5: Configure for Your Board

Just switch between the Project Environments

### Step 6: Build and Upload

1. Connect your ESP32-S3 board via USB-C
2. Press **Ctrl+Alt+U** (Cmd+Alt+U on Mac)
3. Wait for compilation and upload to complete

**Alternative: Using VS Code Upper Bar**
- Click the **→** (Upload) icon in the upper bar

### Step 7: Verify Upload

1. Press **Ctrl+Shift+P**
2. Type **"PlatformIO: Serial Monitor"**
3. You should see debug output at 115200 baud

---

## 🎯 Touch Calibration

The device features an advanced **3-Round Calibration System** that provides superior touch accuracy through multiple refinement passes.

### What is the 3-Round Calibration System?

Because of Problems with a standard 5 Point calibration, caused by an unknown transformation I implemented my own approach:

- ❌ **No** complex multi-point sampling
- ❌ **No** pixel-perfect alignment required

Instead:
- ✅ **Fully automatic** - System knows if you're left/right/up/down
- ✅ **Zone-based detection** - Touch target, system auto-corrects
- ✅ **Real-time convergence** - Parameters adjust while you touch
- ✅ **3-round refinement** - Progressive accuracy improvement
- ✅ **13 total steps** - Complete calibration with multiple passes
- ✅ **Fallback system** - Confirmation dialog with auto-revert

📖 **[Read full documentation](docs/Suthes_Method_Touch_Calibration.md)**

### On-Device Calibration (Recommended)

#### How the 3-Round System Works

**Round 1: Full Calibration (5 steps)**
1. **Offset X** - Touch red dot in center, system auto-adjusts horizontal offset
2. **Offset Y** - Touch red dot in center, system auto-adjusts vertical offset  
3. **Rotation** - Touch red dot on vertical line, system auto-adjusts rotation
4. **Scale X** - Touch red dot on right edge, system auto-adjusts horizontal scale
5. **Scale Y** - Touch red dot on bottom edge, system auto-adjusts vertical scale

**Round 2: Refinement (5 steps)**
- **Refine Offset X** - Ultra-fine horizontal adjustment
- **Refine Offset Y** - Ultra-fine vertical adjustment
- **Refine Rotation** - Ultra-fine angle adjustment
- **Refine Scale X** - Ultra-fine horizontal scale
- **Refine Scale Y** - Ultra-fine vertical scale

**Round 3: Final Offset (2 steps)**
- **Final Offset X** - Ultimate horizontal precision
- **Final Offset Y** - Ultimate vertical precision

**Summary Step**
- **Calibration complete!** - Touch to exit and return to settings

#### Step-by-Step Guide

1. **Access Calibration:**
   - Go to **Settings → Touch Calibration**
   - Read on-screen instructions

2. **Follow the 3-Round Process:**
   - **Round 1**: Complete calibration (5 steps, each with Coarse → Fine)
   - **Round 2**: Refinement of all parameters (5 ultra-fine steps)
   - **Round 3**: Final offset adjustment (2 maximum precision steps)
   - **Just touch and hold** the red dot or line as shown
   - System adjusts parameters **automatically**
   - Release when text says "OK! Release finger"

3. **Confirmation:**
   - Calibration is **immediately applied** for testing
   - On next boot, a **confirmation dialog** appears
   - Choose **Keep** or **Revert** (10-second auto-revert if no action)
   - Once confirmed, calibration is permanent

#### 🎯 How the 3-Round System Works

The 3-Round Calibration System uses **progressive refinement** to achieve optimal accuracy:

```
Round 1: Full Calibration (Coarse → Fine)
┌─────────────────────────────────────────┐
│ Step 1: Offset X    (Coarse → Fine)    │
│ Step 2: Offset Y    (Coarse → Fine)    │
│ Step 3: Rotation    (Coarse → Fine)    │
│ Step 4: Scale X     (Coarse → Fine)    │
│ Step 5: Scale Y     (Coarse → Fine)    │
└─────────────────────────────────────────┘

Round 2: Refinement (Ultra-Fine)
┌─────────────────────────────────────────┐
│ Step 6:  Refine Offset X  (Ultra-Fine) │
│ Step 7:  Refine Offset Y  (Ultra-Fine) │
│ Step 8:  Refine Rotation  (Ultra-Fine) │
│ Step 9:  Refine Scale X   (Ultra-Fine) │
│ Step 10: Refine Scale Y   (Ultra-Fine) │
└─────────────────────────────────────────┘

Round 3: Final Offset (Maximum Precision)
┌─────────────────────────────────────────┐
│ Step 11: Final Offset X (Max Precision)│
│ Step 12: Final Offset Y (Max Precision)│
└─────────────────────────────────────────┘
```

**The 3-round calibration process:**
1. **Round 1**: Establishes baseline calibration with coarse + fine passes
2. **Round 2**: Refines all parameters with ultra-fine adjustments
3. **Round 3**: Achieves maximum precision with final offset adjustments
4. **Progressive accuracy**: Each round builds upon the previous for optimal results
5. **Origin Correction**: A hardcoded origin correction is added to compensate standard origin offset

This creates a **multi-pass refinement system** that guarantees superior calibration accuracy!

#### Testing Calibration

**Best method:** Use the **Preset Editor** keyboard

1. Go to **Settings → Edit Presets**
2. Select any preset
3. Try typing on the on-screen keyboard
4. Test all screen regions (center, edges, corners)
5. If touch feels off → Just re-run calibration (takes 45-65 seconds for full 3-round process!)

**Why the keyboard?**
- Small keys in all screen regions
- Immediate visual feedback
- Most challenging UI element - if keyboard works, everything works!

**Calibration Timing:**
- **Round 1**: 20-30 seconds (5 steps × 4-6 seconds each)
- **Round 2**: 15-25 seconds (5 refinement steps × 3-5 seconds each)  
- **Round 3**: 6-10 seconds (2 final offset steps × 3-5 seconds each)
- **Total**: 45-65 seconds for complete 3-round calibration

### Factory Reset to Defaults

If your device becomes unusable due to bad calibration, you can reset to factory defaults:

#### Option 1: Via Settings (if touch still works)
1. Go to **Settings → Touch Calibration**
2. The calibration will use factory defaults during the process
3. Complete the 3-round calibration process (13 steps total)

#### Option 2: Emergency Code Reset
If touch is completely broken:

1. Open `src/main.cpp` around line 125
2. **Uncomment** the emergency reset line:
   ```cpp
   // === NOTFALL: Touch Calibration Reset ===
   resetTouchCalibrationToDefaults();  // ← Remove the //
   ```
3. Flash to device
4. **Re-comment** the line and flash again

### Manual Default Adjustment (Advanced)

To change factory default values, edit **`src/hardware/display/lvgl_driver.cpp`** line 27-34:

```cpp
static const float DEFAULT_CAL_MATRIX[7] = {
    0.0f,    // [0] offset_x
    0.85f,   // [1] scale_x - Try 0.80-1.05
    0.0f,    // [2] shear_xy - Usually 0.0
    0.0f,    // [3] offset_y
    0.0f,    // [4] shear_yx - Usually 0.0
    1.0f,    // [5] scale_y - Try 0.80-1.05
    1.0f     // [6] divisor - Leave at 1.0
};
```

**Tested Defaults:**
 are saved in the Config.h

> **Recommendation:** Use the automatic 3-round calibration instead of manual adjustment. It provides optimal, device-specific calibration in 45-65 seconds with superior accuracy through progressive refinement!

---

## ⚙️ Adding Custom Presets

You can edit presets on-device via **Settings → Edit Presets**, but the keyboard can be challenging to use. For easier customization, edit presets before compilation.

### Step 1: Locate Preset File

Edit **`src/data/tcg_presets.cpp`**

### Step 2: Add Your Custom Game

**Example: Customizing Slot 8**

```cpp
// ---- SLOT 8: Custom ----
strncpy(TCG_PRESETS[7].name, "Vanguard", sizeof(TCG_PRESETS[7].name) - 1);
TCG_PRESETS[7].starting_life = 5;
TCG_PRESETS[7].small_step = 1;
TCG_PRESETS[7].large_step = 2;
```

**Field Explanation:**
- `name`: Display name (max 15 characters)
- `starting_life`: Initial life total (1-9999)
- `small_step`: Small increment (tap)
- `large_step`: Large increment (swipe)

### Common TCG Presets

**Cardfight!! Vanguard:**
```cpp
strncpy(TCG_PRESETS[7].name, "Vanguard", sizeof(TCG_PRESETS[7].name) - 1);
TCG_PRESETS[7].starting_life = 5;
TCG_PRESETS[7].small_step = 1;
TCG_PRESETS[7].large_step = 2;
```

**Digimon Card Game:**
```cpp
strncpy(TCG_PRESETS[8].name, "Digimon", sizeof(TCG_PRESETS[8].name) - 1);
TCG_PRESETS[8].starting_life = 5;  // Security cards
TCG_PRESETS[8].small_step = 1;
TCG_PRESETS[8].large_step = 2;
```

**Star Wars Unlimited:**
```cpp
strncpy(TCG_PRESETS[9].name, "Star Wars", sizeof(TCG_PRESETS[9].name) - 1);
TCG_PRESETS[9].starting_life = 30;
TCG_PRESETS[9].small_step = 1;
TCG_PRESETS[9].large_step = 5;
```

---

## 📖 Usage Guide

### Basic Controls

#### Main Screen (Single Player)

**Life Adjustment:**
- **Tap Top Half:** Increase life by small step
- **Tap Bottom Half:** Decrease life by small step
- **Swipe Bottom to Top:** Increase by large step
- **Swipe Top to Bottom:** Decrease by large step

**Menu Access:**
- **Long press Center:** Open contextual menu
- **Swipe Right:** Previous page in contextual menu
- **Swipe Left:** Next page in contextual menu

**Timer:**
- **Tap Timer:** Start/pause
- **Long Press Timer:** Reset

**Simple Counters (if enabled):**
- **Tap Counter:** Increment by 1
- **Long Press Counter 0.5s:** Decrement by 1
- **Long Press Counter 1.5s:** Reset Counter

#### Main Screen (Two Player)

**Life Adjustment:**
- **Tap Your Half:** Small adjustment
- **Swipe Your Half:** Large adjustment

**Menu Access:**
- **Long press Center:** Open contextual menu

**Timer:**
- **Tap Timer:** Start/pause
- **Long Press Timer:** Reset

### Menu Navigation

#### Contextual Menu (Page 1)

- **⚙️ Settings Icon:** Open settings menu
- **1P/2P Toggle:** Switch between one and two player modes
- **🔄 Reset Icon:** Reset all life counters to starting values
- **📋 Presets Icon:** Quick-select game presets

#### Contextual Menu (Page 2)

- **🎲 Dice Icon (D):** Open dice selection menu
- **🪙 Coin Icon (C):** Instant coin flip
- **History:** Open the History

#### Settings Menu

**Main Settings:**
- **Start Life:** Adjust starting life and step sizes for current session
- **Power Settings →** Power management submenu
- **Audio Settings →** Sound configuration submenu
- **Timer Settings →** Timer mode and round time submenu
- **Counters →** Simple counters configuration submenu
- **Edit Presets:** Customize game presets
- **Touch Calibration:** On-device touch calibration
- **Swipe to Close:** Enable/disable swipe gestures for menus
- **Restart:** Reboot device
- **Battery Display:** Real-time battery percentage and icon

**Power Settings Submenu:**
1. **Brightness:** Adjust display brightness (0-100%)
2. **Auto-Dim:** Auto-dim after inactivity (ON/OFF, configurable timeout)
3. **Sleep:** Display sleep after inactivity (ON/OFF, configurable timeout)
4. **Low Battery Dimming:** Automatic dimming at ≤15% battery (ON/OFF)

**Audio Settings Submenu:**
1. **Audio:** Master audio toggle (ON/OFF)
2. **Volume:** Adjust volume (0-100%)
3. **Sound:** Select alert sound (4 options) used by Countdown Timer

**Timer Settings Submenu:**
1. **Timer:** Show/hide timer on main screen (ON/OFF)
2. **Timer Mode:** Stopwatch or Countdown
3. **Round Time:** Set countdown duration (1-999 minutes, Countdown mode only)

**Counters Settings Submenu:**
1. **Top-Left Counter:** Enable/disable counter in top-left corner
2. **Top-Right Counter:** Enable/disable counter in top-right corner
3. **Bottom-Left Counter:** Enable/disable counter in bottom-left corner
4. **Bottom-Right Counter:** Enable/disable counter in bottom-right corner

### Advanced Features

#### Using the Dice Roller

1. **Open Contextual Menu**: Long press center of screen
2. **Navigate to Page 2**: Swipe left
3. **Select Dice Icon**: Tap the "D" symbol
4. **Choose Dice Type**: Select D4, D6, D8, D10, D12, D20, or D100
5. **View Result**: Number displayed
6. **Close**: Tap outside or swipe right (if enabled)

#### Using the Coin Flip

1. **Open Contextual Menu**: Long press center
2. **Navigate to Page 2**: Swipe left
3. **Select Coin Icon**: Tap the "C" symbol
4. **View Result**: Heads or Tails displayed

#### Configuring Timer Modes

**Stopwatch Mode:**
1. Settings → Timer Settings → Timer Mode → Stopwatch
2. Timer counts up from 00:00
3. Tracks total game duration
4. No time limit

**Countdown Mode:**
1. Settings → Timer Settings → Timer Mode → Countdown
2. Settings → Timer Settings → Round Time → Enter duration (1-999 minutes)
3. Timer counts down to 00:00
4. Audio alert when time expires
5. Automatic pause at 00:00

#### Using the Simple Counters System

The Simple Counters system provides 4 configurable auxiliary counters for tracking additional game metrics:

**Enable Counters:**
1. Settings → Counters → Enable desired counters (Top-Left, Top-Right, Bottom-Left, Bottom-Right)
2. Counters appear in their respective corners

**Usage:**
- **Short Tap:** Increment by 1
- **Medium Hold (0.5-1.5s):** Decrement by 1 (visual feedback: yellow text)
- **Long Hold (>1.5s):** Reset to 0 (visual feedback: red text)
- **Theme Integration:** Counters adapt to selected theme colors
- **Semi-Transparent:** 80% opacity with gradient effects for better integration

**Common Uses:**
- **MTG**: Commander damage, poison counters, storm count, energy counters
- **Pokémon**: Energy counters, damage counters, special conditions
- **Yu-Gi-Oh!**: Life Point modifiers, special counters
- **General**: Any auxiliary tracking needed during gameplay

**Configuration:**
- All counter settings are centralized in `config.h`
- Adjustable positioning, size, opacity, colors, and timing
- Counters reset automatically when main life counter is reset

#### Editing Custom Presets

**Using Preset Editor (On-Device):**
1. Settings → Edit Presets
2. Select any preset (Custom 8, Custom 9, Custom 10 recommended)
3. Enter preset name (max 15 characters, on-screen keyboard)
4. Set starting life (1-9999)
5. Set small step (tap increment)
6. Set large step (swipe increment)
7. Save preset (automatically saved to NVS)

**Adding Presets Before Compilation** (Recommended):

See [Adding Custom Presets](#-adding-custom-presets) section for editing `tcg_presets.cpp` before building.

#### Power Management Tips

**Extend Battery Life:**
1. Enable Auto-Dim (Settings → Power Settings → Auto-Dim → ON)
2. Enable Sleep (Settings → Power Settings → Sleep → ON)
3. Reduce brightness (Settings → Power Settings → Brightness → 30-50%)


**Battery Protection:**
- Low Battery Dimming activates automatically at ≤15%
- Wake device by pressing power button
- Critical Battery Protection triggers at ≤2%
- Device enters deep sleep to prevent battery damage


**Charging:**
- Connect USB-C cable
- Device can operate while charging
- Low Battery Dimming disabled during charging 
- Battery percentage may show 0% while charging (normal behavior)

---

## 🔋 Power Management

### Overview

The Life Puck includes comprehensive power management to extend battery life and protect the battery from damage.

### Features

#### Auto-Dim
- **Function:** Reduces display to 25% brightness after inactivity
- **Default:** 60 seconds
- **Configuration:** Power Settings → Auto-Dim
- **Benefits:** Extends battery life during pauses in gameplay

#### Display Sleep
- **Function:** Turns off display completely after extended inactivity
- **Default:** 300 seconds (5 minutes)
- **Configuration:** Power Settings → Sleep
- **Wake:** Touch screen anywhere

#### Low Battery Dimming
- **Function:** Forces display to 5% brightness at ≤15% battery
- **Automatic:** Activates when battery drops to critical levels
- **Restoration:** Automatically restores brightness when battery > 15%
- **USB Detection:** Disabled when charging (voltage < 1V)
- **Configuration:** Power Settings → Low Battery Dimming

#### Critical Battery Protection

**Purpose:** Prevents permanent battery damage from deep discharge

**How It Works:**
1. Battery drops to ≤2%
2. 2-second verification timer starts
3. After 2 seconds, voltage is re-checked
4. If voltage is between 1V and 3.3V → Deep sleep activated
5. If voltage < 1V (USB charging) or > 3.3V → Protection canceled

**Smart USB Detection:**
- Voltage < 1V → USB charging detected, no shutdown
- 30-second timeout handles power switch toggling during USB charging
- 10-second boot grace period allows voltage readings to stabilize

**Why This Matters:**
- The original Life Puck had no automatic battery protection
- Without this feature, battery can drain to 2.7V, causing permanent damage
- This system ensures battery longevity and prevents costly replacements

**Wake from Deep Sleep:**
- connect USB-C cable
- Press and hold power button for 2 seconds (non C Model)

### Best Practices

**For Maximum Battery Life:**
1. Enable Auto-Dim and Sleep
2. Set brightness to 30-50%
3. Enable Low Battery Dimming
4. Charge battery before it reaches 15%

**For Maximum Performance:**
1. Set brightness to 70-100%
2. Disable Auto-Dim
3. Set Sleep timeout to 0 (disabled)
4. Keep Low Battery Dimming enabled (battery protection)


---

## 🐛 Troubleshooting

### Build Errors

**Error: `platform-espressif32` not found**

Solution: Delete .pio folder and rebuild

### Upload Issues

**Device Not Detected**
1. Install CH340 drivers: [Download](http://www.wch-ic.com/downloads/CH341SER_EXE.html)
2. Try a different USB cable (must support data transfer)
3. Hold BOOT button while connecting USB

**Permission Denied (Linux)**

sudo usermod -a -G dialout $USER
Logout and login again


### Display Issues

**Display Shows Garbage**
- Check board variant in `platformio.ini` matches your hardware
- Rebuild with `pio run -t clean && pio run`

**Colors Inverted**
- Edit `src/Display_ST77916.cpp`
- Toggle `LCD_CMD_INVON` / `LCD_CMD_INVOFF`

**Touch Not Working**
- Verify touch controller is CST816
- Check I2C address in `Touch_CST816.cpp` (default: 0x15)
- Adjust calibration (see [Touch Calibration](#-touch-calibration))

### Runtime Issues

**Device Crashes on Startup**

Check serial monitor for error messages

pio device monitor -b 115200


**Settings Not Saving**
- NVS (Non-Volatile Storage) may be corrupted
- Erase flash: `pio run -t erase`
- Re-upload firmware: `pio run -t upload`

**Battery Drains Quickly**
- Reduce display brightness in settings
- Check battery capacity (recommended: ≥ 500mAh)

---



## 👏 Credits

### Original Project
- **jontiritilli** - Original Life Puck project: [github.com/jontiritilli/life-puck](https://github.com/jontiritilli/life-puck)

### This Fork
- **Platinplayer23** - Multi-hardware support, dice roller, coin flip, enhanced timer, Arduino 3.x integration

### Libraries Used
- **LVGL** - Graphics library (v9.3.0)
- **TFT_eSPI** - Display driver
- **ArduinoNvs** - Non-volatile storage
- **pioarduino** - Arduino 3.x ESP32 platform

### Hardware
- **Waveshare** - ESP32-S3 display modules
- **Espressif** - ESP32-S3 MCU

---

## 📄 License

This project inherits the license from the original [jontiritilli/life-puck](https://github.com/jontiritilli/life-puck) repository.

See [LICENSE](LICENSE) for details.

---

## 🔗 Useful Links

### Hardware Documentation
- [ESP32-S3-Touch-LCD-1.85C Wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85C)
- [ESP32-S3-Touch-LCD-1.85 Wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85)
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)

### Software Resources
- [PlatformIO Documentation](https://docs.platformio.org/)
- [LVGL Documentation](https://docs.lvgl.io/9.3/)
- [pioarduino Platform](https://github.com/pioarduino/platform-espressif32)

### Purchase Links
- [Waveshare Official Store](https://www.waveshare.com/)
- [AliExpress Bundle (1.85C + Case + Battery)](https://de.aliexpress.com/item/1005009857920438.html?spm=a2g0o.order_list.order_list_main.10.6a295c5fDaffaT&gatewayAdapt=glo2deu) - **Recommended**

---

## 📞 Support

- **Issues:** [GitHub Issues](https://github.com/YOUR-USERNAME/life-puck/issues)
- **Discussions:** [GitHub Discussions](https://github.com/YOUR-USERNAME/life-puck/discussions)
- **Original Project:** [jontiritilli/life-puck](https://github.com/jontiritilli/life-puck)

---

**Made with ❤️ for the tabletop gaming community**