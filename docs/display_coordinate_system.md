# Display Coordinate System Documentation

## 🎯 Problem Discovery
The ESP32-S3 Touch LCD 1.85" round display has a **coordinate system offset**:
- **Theoretical center**: (180, 180) - calculated from 360x360 resolution
- **Actual visual center**: **(162, 162)** - confirmed via crosshair calibration
- **Offset**: 18 pixels in both X and Y directions

## 📐 Technical Details

### LVGL Coordinate System
- **Resolution**: 360x360 pixels
- **Origin**: (0, 0) at top-left corner
- **Display Type**: Circular, with visible area offset from coordinate origin
- **True Center**: (162, 162) - where UI elements are visually centered

### How the True Center Was Found
1. Created cyan crosshair using `lv_obj_align(LV_ALIGN_CENTER)` - LVGL's automatic centering
2. Created yellow crosshair at (180, 180) - manual positioning at theoretical center
3. Disabled scrolling to enable precise touch input
4. Moved yellow crosshair step-by-step until it aligned with cyan crosshair
5. **Result**: Both crosshairs aligned at **(162, 162)**

## 🔧 Touch Calibration Implementation

### Calibration Method: Suthe's Method

Since I couldn't find any similar approach in existing implementations, I'm calling this **"Suthe's Method"** - an interactive zone-based calibration technique.

**See full documentation:** [Suthe's Method Documentation](Suthes_Method_Touch_Calibration.md)

### Quick Overview
The calibration uses **invisible detection zones** with **automatic parameter adjustment** for a fully guided user experience.

### User Workflow
1. **Step 1: Offset X** - Touch red vertical line in center, system auto-adjusts until centered
2. **Step 2: Offset Y** - Touch red horizontal line in center, system auto-adjusts until centered
3. **Step 3: Rotation** (Optional) - Touch red dot on diagonal, system auto-adjusts angle
4. **Step 4: Scale X** - Touch red dot on right edge, system auto-adjusts scale
5. **Step 5: Scale Y** - Touch red dot on bottom edge, system auto-adjusts scale
6. **Confirmation dialog** - Keep/Revert with 10-second auto-revert

### Key Features
- ✅ **Fully Automatic**: No pixel-perfect alignment needed
- ✅ **Zone-Based Detection**: System knows if you're left/right/up/down
- ✅ **Real-Time Adjustment**: Parameters converge while you touch
- ✅ **Two-Pass System**: Coarse (fast) → Fine (precise)
- ✅ **Self-Correcting**: Feedback loop ensures convergence

### 7-Parameter Matrix Transformation
The calibration produces a **7-parameter affine transformation matrix**:
- **Offset (C, F)** - Translation correction
- **Scale (A, E)** - Scaling correction
- **Shear (B, D)** - Rotation/distortion correction (optional)
- **Divisor** - Normalization factor

**Matrix Layout**:
```cpp
matrix[0] = C  // X offset
matrix[1] = A  // X scale
matrix[2] = B  // X shear (for rotation)
matrix[3] = F  // Y offset
matrix[4] = D  // Y shear (for rotation)
matrix[5] = E  // Y scale
matrix[6] = 1.0f  // Divisor
```

**How Suthe's Method Calibrates Each Parameter:**
- **C, F (Offset)** - Zone-based detection (left/right, up/down)
- **A, E (Scale)** - Edge target with incremental increase
- **B, D (Rotation)** - Diagonal target with angle adjustment (optional)

### Touch Transformation (Applied in Real-Time)
After calibration, all touch input is transformed using the full matrix:
```cpp
float cal_x = (raw_x * g_cal_matrix[1]) + (raw_y * g_cal_matrix[2]) + g_cal_matrix[0];
float cal_y = (raw_x * g_cal_matrix[4]) + (raw_y * g_cal_matrix[5]) + g_cal_matrix[3];
cal_x /= g_cal_matrix[6];
cal_y /= g_cal_matrix[6];
```

This **7-parameter transformation** corrects for:
- Hardware misalignment
- Non-linear touch panel distortion ("warped" touch response)
- Rotation and shear effects

## 📊 Coordinate Systems

### Touch Sensor Coordinate System
- **Range**: 0-360 (12-bit values from CST816 touch controller)
- **Origin**: Top-left of physical touch panel
- **Note**: May have slight offset from display coordinates

### Display Coordinate System
- **Range**: 0-360 pixels
- **Origin**: Top-left of LVGL canvas
- **Visual Center**: (162, 162) - confirmed via LVGL's alignment functions
- **Theoretical Center**: (180, 180) - incorrect for this display

### Calibration Goal
Map touch sensor coordinates to display coordinates such that:
- Touching the visual center → reports (162, 162)
- Touch accuracy is uniform across the entire screen
- Scale and offset compensate for any hardware misalignment

## 🎯 Default Calibration Values

### Factory Defaults (7-Parameter Matrix)
**Now defined in `src/config.h` for easy editing:**
```cpp
#define TOUCH_CAL_DEFAULT_OFFSET_X 0.0f
#define TOUCH_CAL_DEFAULT_SCALE_X 0.85f
#define TOUCH_CAL_DEFAULT_SHEAR_XY 0.0f
#define TOUCH_CAL_DEFAULT_OFFSET_Y 0.0f
#define TOUCH_CAL_DEFAULT_SHEAR_YX 0.0f
#define TOUCH_CAL_DEFAULT_SCALE_Y 0.90f
#define TOUCH_CAL_DEFAULT_DIVISOR 1.0f
```
These values provide reasonable touch accuracy on uncalibrated systems:
- **scale_x = 0.85**: Empirically determined for this hardware (horizontal compression)
- **scale_y = 0.90**: Vertical scaling adjustment (was 1.0, now 0.90 for better accuracy)
- **All shear/offset = 0.0**: No distortion compensation by default
- Serve as fallback if calibration is rejected or times out
- **Can be edited in config.h** without modifying code

### Calibration Behavior
- **During Calibration**: Transformation reset to identity matrix (1:1 raw values)
  ```cpp
  float identity_matrix[7] = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
  ```
- **After Successful Calibration**: Full 7-parameter matrix stored in NVS
  - Stored as 7 separate float values (`touch_cal_0` through `touch_cal_6`)
  - Applied immediately
  - Pending confirmation flag set
- **After Rejected/Timeout**: Reset to factory defaults
- **On System Boot**: 
  - Load 7-parameter matrix from NVS if valid
  - Show confirmation dialog if pending
  - Use factory defaults if no calibration exists

## ✅ Current Status
- **Touch Calibration**: ✅ Fully functional with Suthe's Method (zone-based interactive calibration)
- **UI Positioning**: ✅ All menus and screens properly centered at (162, 162)
- **LVGL Integration**: ✅ `lv_obj_align(LV_ALIGN_CENTER)` automatically uses correct center
- **NVS Storage**: ✅ Full 7-parameter matrix persists across reboots
- **Two-Pass System**: ✅ Coarse (fast) → Fine (precise) adjustment for optimal speed and accuracy
- **Rotation Calibration**: ✅ Optional rotation step for advanced correction
- **Confirmation Dialog**: ✅ 10-second timeout with Keep/Revert options
- **Power Management**: ✅ Auto-dim and sleep suspended during calibration (no interruptions)
- **Centralized Config**: ✅ All calibration defaults in config.h (easy to edit)
- **Force Reset Flags**: ✅ FORCE_DEFAULT_CALIBRATION to reset to config.h values

## 📁 Related Files
- `src/config.h` - **Touch calibration defaults** (edit here to change defaults!)
- `src/ui/screens/settings/touch_calibration.cpp` - Main calibration implementation
- `src/ui/screens/settings/touch_calibration.h` - Header with function declarations
- `src/hardware/display/lvgl_driver.cpp` - Touch transformation application (loads defaults from config.h)
- `src/hardware/touch/touch_cst816.cpp` - Raw touch data acquisition
- `src/hardware/system/power_management.cpp` - Power suspend during calibration (prevents auto-dim/sleep)
- `src/data/constants.h` - Screen dimensions and constants

## 🔍 Implementation Notes

### Why 7-Parameter Matrix vs Simple Scale/Offset?
The **7-parameter affine transformation** provides superior accuracy because:

1. **Non-Linear Distortion**: Touch panels often have "warped" response
   - Simple scale/offset assumes perfect linearity
   - Real-world panels show position-dependent distortion
   - Shear parameters (B, D) can compensate for rotation and distortion

2. **Zone-Based Calibration**: Suthe's Method approach
   - User-friendly (no pixel-perfect alignment)
   - Automatic convergence via feedback loop
   - Calibrates offset and scale independently
   - Optional rotation step for advanced correction

3. **Example Problem Solved**:
   ```
   User reported: "Touch recognized further right than pressed"
   Cause: Incorrect X scaling (scale_x too high)
   Solution: Scale X calibration step adjusts automatically
   ```

---

**Last Updated**: January 2025  
**Status**: ✅ PRODUCTION READY - Interactive zone-based touch calibration system
