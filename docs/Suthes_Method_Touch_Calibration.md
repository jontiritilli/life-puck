# Touch Calibration System - 3-Round Calibration

## Introduction

This document describes the **3-Round Calibration System** - an advanced touch calibration technique that provides superior accuracy through multiple refinement passes.

The system evolved from the original "Suthe's Method" (zone-based calibration) into a more sophisticated approach that ensures optimal touch accuracy through three distinct calibration rounds.

---

## What is the 3-Round Calibration System?

The **3-Round Calibration System** is an advanced calibration approach that uses multiple refinement passes to achieve optimal touch accuracy:

### Core Concept

The system performs calibration in three distinct rounds:

1. **Round 1: Full Calibration** - Complete coarse + fine calibration of all parameters
2. **Round 2: Refinement** - Fine-tuning of all parameters for improved accuracy  
3. **Round 3: Final Offset** - Final offset adjustment for perfect alignment

Each round uses the same interactive approach:
1. **Shows a target** (line or dot)
2. **User touches the target** (doesn't need to be pixel-perfect)
3. **System detects deviation** and auto-adjusts parameters
4. **Loop repeats** until touch lands in target zone
5. **Convergence achieved** → Lock and advance to next step

This creates a **multi-pass refinement system** that ensures optimal calibration accuracy.

---

## Technical Implementation

### 3-Round Calibration Process

The system calibrates 5 parameters through three distinct rounds:

#### Round 1: Full Calibration (Coarse + Fine)
1. **Offset X** - Horizontal offset correction (Coarse → Fine)
2. **Offset Y** - Vertical offset correction (Coarse → Fine)
3. **Rotation** - Rotational alignment (Coarse → Fine)
4. **Scale X** - Horizontal scaling correction (Coarse → Fine)
5. **Scale Y** - Vertical scaling correction (Coarse → Fine)

#### Round 2: Refinement
- **Refine Offset X** - Fine-tuning horizontal offset
- **Refine Offset Y** - Fine-tuning vertical offset
- **Refine Rotation** - Fine-tuning rotation angle
- **Refine Scale X** - Fine-tuning horizontal scale
- **Refine Scale Y** - Fine-tuning vertical scale

#### Round 3: Final Offset
- **Final Offset X** - Ultimate horizontal precision
- **Final Offset Y** - Ultimate vertical precision

### Multi-Pass System

Each step uses different precision levels:

#### Round 1: Coarse + Fine Passes
- **Coarse Pass**: Large step sizes, quick approach to target
- **Fine Pass**: Small step sizes, high precision tuning
- **Automatic switching** from coarse to fine

#### Round 2: Refinement Pass
- **Ultra-fine step sizes** (50% of fine pass)
- **Double lock frames** for stability
- **Enhanced precision** for all parameters

#### Round 3: Final Pass
- **Maximum precision** offset adjustment
- **Double lock frames** for final stability
- **Perfect alignment** achievement

### Zone-Based Detection

Each calibration step uses **invisible hit-test zones** to detect touch position:

#### Offset X Calibration
```
┌─────────────────────────────┐
│         │         │         │
│  LEFT   │ TARGET  │  RIGHT  │
│  ZONE   │  ZONE   │  ZONE   │
│         │         │         │
└─────────────────────────────┘
```
- **Left zone hit** → `offset_x += step` (shift right)
- **Right zone hit** → `offset_x -= step` (shift left)
- **Target zone hit** → Lock (perfectly centered!)

#### Offset Y Calibration
```
┌─────────────────────────────┐
│        TOP ZONE             │
├─────────────────────────────┤
│      TARGET ZONE            │
├─────────────────────────────┤
│      BOTTOM ZONE            │
└─────────────────────────────┘
```
- **Top zone hit** → `offset_y += step` (shift down)
- **Bottom zone hit** → `offset_y -= step` (shift up)
- **Target zone hit** → Lock (perfectly centered!)

#### Rotation Calibration (Optional)
```
┌─────────────────────────────┐
│ LEFT  │  TARGET (UPPER) │RGT│
│ ZONE  │   HALF OF LINE  │ZNE│
│       │        ●        │   │
│       │                 │   │
│       │                 │   │
└─────────────────────────────┘
```
- **Visual**: Vertical red line with red dot in upper quarter
- **Target zone**: Entire upper half of the line (180px tall × 5px wide)
- **Left zone hit** → Rotate counter-clockwise
- **Right zone hit** → Rotate clockwise
- **Target zone hit** → Lock (perfect rotation!)
- **Note**: Large target area makes rotation calibration much easier

#### Scale X Calibration
```
┌─────────────────────────────┐
│                             │
│  INCREASE ZONE       ● TGT  │
│  (Touch registers left)     │
│                             │
└─────────────────────────────┘
```
- **Left zone hit** → `scale_x += step` (expand horizontally)
- **Target zone hit** → Lock (perfect scale!)

#### Scale Y Calibration
```
┌─────────────────────────────┐
│     INCREASE ZONE           │
│  (Touch registers high)     │
├─────────────────────────────┤
│          ● TARGET           │
└─────────────────────────────┘
```
- **Top zone hit** → `scale_y += step` (expand vertically)
- **Target zone hit** → Lock (perfect scale!)

---

## Advantages of the 3-Round Calibration System

### ✅ User Experience
- **No pixel-perfect alignment required** - Just touch and hold
- **Fully automatic** - System adjusts parameters without user input
- **Visual feedback** - Clear "OK! Release finger" when locked
- **Superior accuracy** - 3-round refinement ensures optimal precision
- **Intuitive** - No technical knowledge needed
- **Progressive refinement** - Each round improves accuracy

### ✅ Technical Benefits
- **Multi-pass refinement** - 3 rounds ensure optimal calibration
- **Self-correcting** - Feedback loop ensures convergence
- **Axis decoupling** - X and Y calibrated independently
- **No complex math during calibration** - Simple hit-tests only
- **Robust** - Works even with severely miscalibrated starting values
- **Real-time** - Parameters update at 20 Hz (50ms timer)
- **Enhanced precision** - Round 2 and 3 provide ultra-fine adjustments

### ✅ Implementation Advantages
- **No multi-point sampling** - No need for 9 precise touch points
- **No least-squares calculation** - No matrix inversion or complex math
- **Low memory footprint** - No sample buffers or large arrays
- **Fast execution** - Hit-tests are computationally cheap
- **LVGL native** - Uses standard `lv_obj_hit_test()` API
- **Progressive accuracy** - Each round builds upon the previous

---

## Disadvantages and Limitations

### ⚠️ Potential Issues

#### 1. Convergence Speed
- **Problem:** May oscillate near target zone in fine pass
- **Impact:** Can take a few extra seconds to lock
- **Mitigation:** Frame counter requires 10 consecutive hits to lock

#### 2. Non-Linear Distortion
- **Problem:** Method assumes linear transformation (scale + offset)
- **Impact:** May not correct severe non-linear touch panel warping
- **Mitigation:** Shear parameters (B, D) in matrix can be manually adjusted if needed

#### 3. Rotation Calibration Complexity
- **Problem:** Rotation step requires understanding of center-based rotation
- **Impact:** User may not understand "touch on line" instruction
- **Mitigation:** Rotation step is optional and can be skipped

#### 4. Dependency on lv_obj_hit_test()
- **Problem:** Requires LVGL's hit-test to work correctly
- **Impact:** Transparent objects (opacity = 0) cannot be detected in LVGL v9
- **Mitigation:** Use `lv_btn_create()` with `LV_OPA_TRANSP` for invisible zones

#### 5. No Cross-Axis Correction
- **Problem:** X-offset doesn't account for Y-position variations
- **Impact:** If touch panel has position-dependent offset, calibration may be suboptimal
- **Mitigation:** Use shear parameters (B, D) for advanced correction

---

## Implementation Details

### Code Structure

**File:** `src/ui/screens/settings/touch_calibration.cpp`

#### Key Components:

1. **State Machine:**
   ```cpp
   typedef enum {
       CAL_STEP_OFFSET_X,
       CAL_STEP_OFFSET_Y,
       CAL_STEP_ROTATION,  // Optional
       CAL_STEP_SCALE_X,
       CAL_STEP_SCALE_Y,
       CAL_STEP_SUMMARY,
       CAL_STEP_FINISHED
   } calibration_step_t;
   ```

2. **Zone Objects:**
   ```cpp
   static lv_obj_t* zone1 = nullptr;        // Correction zone 1 (left/top)
   static lv_obj_t* zone2 = nullptr;        // Correction zone 2 (right/bottom)
   static lv_obj_t* zone_target = nullptr;  // Target zone (lock zone)
   ```

3. **Core Logic:**
   ```cpp
   static void process_calibration_logic(lv_timer_t* timer) {
       // Read raw touch data
       Touch_Read_Data();
       
       // Calculate calibrated position
       // Perform hit-tests on zones
       // Adjust parameters based on which zone was hit
       // Lock if target zone hit for 10 consecutive frames
   }
   ```

### Matrix Application

The calibration matrix is a **7-parameter affine transformation**:

```cpp
float g_cal_matrix[7] = {
    offset_x,  // [0] C - X translation
    scale_x,   // [1] A - X scaling
    shear_xy,  // [2] B - X shear from Y
    offset_y,  // [3] F - Y translation
    shear_yx,  // [4] D - Y shear from X
    scale_y,   // [5] E - Y scaling
    divisor    // [6] - Normalization
};
```

**Transformation formula:**
```cpp
cal_x = (raw_x * A) + (raw_y * B) + C
cal_y = (raw_x * D) + (raw_y * E) + F
cal_x /= divisor
cal_y /= divisor
```

During calibration, Suthe's Method adjusts **C, F, A, E** (and optionally **B, D** for rotation).

---

## Comparison with Traditional Methods

### Traditional Multi-Point Calibration

**Typical Approach:**
1. Show 9 target points
2. User touches each point precisely
3. System samples raw coordinates
4. Least-squares fit calculates transformation matrix

**Pros:**
- Mathematically optimal (minimizes total error)
- Corrects non-linear distortion
- Single-shot calibration (no iteration)

**Cons:**
- Requires pixel-perfect alignment (difficult on small screens)
- User must touch 9 points accurately
- Complex math (matrix inversion, least-squares)
- More code, higher memory footprint

### Suthe's Method

**Approach:**
1. Show 1 target at a time
2. User touches target (approximate)
3. System detects deviation and auto-corrects
4. Repeat for 4-5 steps

**Pros:**
- No precise alignment needed
- Fully automatic adjustment
- Simple implementation (just hit-tests)
- Fast and intuitive user experience
- Guaranteed convergence (feedback loop)

**Cons:**
- May not correct severe non-linear distortion
- Takes slightly longer than single-shot methods
- Requires multiple user interactions (4-5 touches)

---

## User Workflow

### 3-Round Calibration Process

The calibration consists of **3 rounds** with **13 total steps**:

#### Round 1: Full Calibration (5 steps, each with Coarse + Fine)

**Step 1: Offset X (Horizontal Centering)**
1. **Red dot** appears in center
2. **User touches and holds** the dot
3. **System detects** if touch is left or right of center
4. **Offset adjusts automatically** (Coarse → Fine)
5. **Text changes** to "OK! Release finger"
6. **User releases** → Advances to next step

**Step 2: Offset Y (Vertical Centering)**
1. **Red dot** appears in center
2. **User touches and holds** the dot
3. **System detects** if touch is above or below center
4. **Offset adjusts automatically** (Coarse → Fine)
5. **Text changes** to "OK! Release finger"
6. **User releases** → Advances to next step

**Step 3: Rotation**
1. **Red vertical line** appears with **red dot in upper half**
2. **User touches and holds** the upper half of the line (near the dot)
3. **System detects** if touch is left/right of line
4. **Angle adjusts automatically** (Coarse → Fine)
5. **Text changes** to "OK! Release finger"
6. **User releases** → Advances to next step

**Step 4: Scale X (Horizontal Scaling)**
1. **Red dot** appears on **right edge**
2. **User touches and holds** the dot
3. **Scale adjusts automatically** (Coarse → Fine)
4. **Text changes** to "OK! Release finger"
5. **User releases** → Advances to next step

**Step 5: Scale Y (Vertical Scaling)**
1. **Red dot** appears on **bottom edge**
2. **User touches and holds** the dot
3. **Scale adjusts automatically** (Coarse → Fine)
4. **Text changes** to "OK! Release finger"
5. **User releases** → Round 1 complete!

#### Round 2: Refinement (5 steps)

**Steps 6-10: Refine All Parameters**
- **Refine Offset X** - Ultra-fine horizontal adjustment
- **Refine Offset Y** - Ultra-fine vertical adjustment
- **Refine Rotation** - Ultra-fine angle adjustment
- **Refine Scale X** - Ultra-fine horizontal scale
- **Refine Scale Y** - Ultra-fine vertical scale

Each step uses the same process as Round 1, but with enhanced precision and double lock frames.

#### Round 3: Final Offset (2 steps)

**Step 11: Final Offset X**
- **Ultimate horizontal precision** adjustment
- **Maximum accuracy** for perfect alignment

**Step 12: Final Offset Y**
- **Ultimate vertical precision** adjustment
- **Maximum accuracy** for perfect alignment

#### Step 13: Summary
- **Calibration complete!** message
- **Touch to exit** and return to settings

---

## Technical Notes

### Why This Works

The method exploits a fundamental property of affine transformations:

**Offset calibration is independent of scale:**
- Adjusting `offset_x` shifts all touches horizontally
- **Zone detection** tells system which direction to shift
- System converges to correct offset regardless of scale

**Scale calibration is independent of offset:**
- Adjusting `scale_x` expands/contracts touches from origin
- By placing target at edge, system can measure if scale is too small
- System increases scale until edge touch registers correctly

### Convergence Guarantee

The method is **guaranteed to converge** because:

1. **Monotonic adjustment** - Parameters only move toward target
2. **Frame-based locking** - Prevents premature lock on noise
3. **Two-pass refinement** - Coarse pass gets close, fine pass perfects
4. **Backtracking** - Stepping back from coarse lock ensures fine pass captures target

### Performance

**Timing (typical for 3-Round System):**
- **Round 1**: 20-30 seconds (5 steps × 4-6 seconds each)
- **Round 2**: 15-25 seconds (5 refinement steps × 3-5 seconds each)
- **Round 3**: 6-10 seconds (2 final offset steps × 3-5 seconds each)
- **Total: 45-65 seconds** for complete 3-round calibration

**Frame rate:** 20 Hz (50ms timer)

**Convergence:** 
- Round 1: 40-120 frames per step
- Round 2: 60-150 frames per step (enhanced precision)
- Round 3: 80-200 frames per step (maximum precision)

---

## Code Example

### Creating Detection Zones

```cpp
// Invisible zone for left-side detection
zone1 = lv_btn_create(cal_screen);
lv_obj_set_size(zone1, (SCREEN_WIDTH / 2) - 3, SCREEN_HEIGHT);
lv_obj_align(zone1, LV_ALIGN_LEFT_MID, 0, 0);
lv_obj_set_style_bg_opa(zone1, LV_OPA_TRANSP, 0);
lv_obj_set_style_border_opa(zone1, LV_OPA_TRANSP, 0);
lv_obj_set_style_outline_opa(zone1, LV_OPA_TRANSP, 0);
lv_obj_add_flag(zone1, LV_OBJ_FLAG_CLICKABLE);
```

### Hit-Test Logic

```cpp
// Calculate calibrated coordinates
float cal_x = (raw_x * scale_x) + offset_x;
float cal_y = (raw_y * scale_y) + offset_y;

lv_point_t p = {(int16_t)cal_x, (int16_t)cal_y};

// Test which zone was hit
bool hit_left = lv_obj_hit_test(zone1, &p);
bool hit_right = lv_obj_hit_test(zone2, &p);
bool hit_target = lv_obj_hit_test(zone_target, &p);

// Adjust based on zone
if (hit_left) {
    offset_x += step_size;  // Shift right
} else if (hit_right) {
    offset_x -= step_size;  // Shift left
} else if (hit_target) {
    lock_frame_counter++;   // Count frames in target
    if (lock_frame_counter >= 10) {
        locked = true;      // Lock achieved!
    }
}
```

---

## Fallback and Confirmation

After calibration completes, the system:

1. **Saves matrix to NVS** with pending confirmation flag
2. **Applies calibration immediately** for testing
3. **Returns to settings menu**

On next boot:

1. **Loads calibrated matrix** from NVS
2. **Shows confirmation dialog** (10-second timeout)
3. **User chooses:**
   - **Keep** → Confirmation flag cleared, calibration permanent
   - **Revert** → Factory defaults restored
   - **Timeout** → Automatic revert to factory defaults

This ensures bad calibrations cannot permanently break the device.

---

## Future Enhancements

Potential improvements to Suthe's Method:

1. **Adaptive step sizes** - Reduce step size as zone is approached
2. **Predictive convergence** - Estimate final value and jump directly
3. **Multi-zone refinement** - Use smaller zones for higher precision
4. **Visual zone indicators** - Show zones during calibration for debugging
5. **Skip rotation** - Make rotation step optional via flag
6. **One-pass mode** - Skip fine pass for faster calibration

---

## Manual Calibration Method (Alternative)

For advanced users or when the automatic 3-round calibration doesn't provide sufficient accuracy, manual calibration is available:

### Manual Calibration Process

1. **Access Manual Mode**: Edit `src/main.cpp` around line 125
2. **Uncomment Emergency Reset**: Remove `//` from `resetTouchCalibrationToDefaults();`
3. **Flash to Device**: Upload the modified firmware
4. **Re-comment Line**: Restore the `//` and flash again

### Manual Default Adjustment

To change factory default values, edit **`src/hardware/display/lvgl_driver.cpp`** lines 27-34:

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
- **ESP32-S3-Touch-LCD-1.85C:** `scale_x = 0.85f, scale_y = 0.90f`
- **ESP32-S3-Touch-LCD-1.85:** `scale_x = 1.0f, scale_y = 1.0f`

### When to Use Manual Calibration

- **Severe touch panel issues** that automatic calibration cannot resolve
- **Custom hardware modifications** requiring specific calibration values
- **Debugging touch problems** during development
- **Fine-tuning** for specific use cases

> **Recommendation:** Always try the automatic 3-round calibration first. Manual calibration should only be used when the automatic system fails to provide adequate results.

---

## Conclusion

The 3-Round Calibration System provides an optimal balance between:
- **User-friendliness** (no precise alignment needed)
- **Technical accuracy** (multi-pass refinement ensures convergence)
- **Implementation simplicity** (zone-based hit-tests only)
- **Superior precision** (3 rounds of progressive refinement)

The system evolved from the original "Suthe's Method" to provide even better accuracy through multiple refinement passes, while maintaining the intuitive user experience.

---

**Author:** Suthe (Pascal)  
**Date:** January 2025  
**Status:** Production Ready  
**License:** Inherited from original Life Puck project
