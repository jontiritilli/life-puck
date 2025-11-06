#pragma once

typedef struct {
    char name[32];
    int starting_life;
    int small_step;
    int large_step;
} TCGPreset;

// Dynamisches Array (max 10)
extern TCGPreset TCG_PRESETS[10];
extern int TCG_PRESET_COUNT;  // <- NICHT const!

void init_presets();
void load_preset();
void save_preset(int index);
TCGPreset get_preset();