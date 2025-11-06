#pragma once
#include "Arduino.h"

// Simple I2S Audio for PCM5101 DAC
#define I2S_DOUT 47
#define I2S_BCLK 48  
#define I2S_LRC  38

// Audio settings
#define AUDIO_VOLUME_MAX 21
// Note: AUDIO_VOLUME_DEFAULT is defined in config.h

// Sound types
typedef enum {
    SOUND_TIMER_FINISH = 0,
    SOUND_TIMER_FINISH_ALT1,
    SOUND_TIMER_FINISH_ALT2,
    SOUND_TIMER_FINISH_ALT3,
    SOUND_BUTTON_CLICK,
    SOUND_SUCCESS,
    SOUND_ERROR,
    SOUND_STARTUP,
    SOUND_COUNT
} sound_type_t;

// Simple audio functions
void simple_audio_init();
void simple_audio_beep(int frequency, int duration_ms);
void simple_audio_play_sound(sound_type_t sound);
void simple_audio_set_volume(int volume);
int simple_audio_get_volume();
void simple_audio_set_enabled(bool enabled);
bool simple_audio_is_enabled();
void simple_audio_set_timer_sound(sound_type_t sound);
sound_type_t simple_audio_get_timer_sound();
void simple_audio_cleanup();

// Wrapper functions for settings compatibility
bool getAudioEnabled();
void toggleAudioEnabled();
int getVolume();
void setVolume(int volume);
bool getSoundEffectsEnabled();
void toggleSoundEffectsEnabled();
