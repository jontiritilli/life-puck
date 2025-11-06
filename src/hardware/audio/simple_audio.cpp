#include "simple_audio.h"
#include "config.h"
#include "driver/i2s.h"
#include "ArduinoNvs.h"
#include "core/state_manager.h"

// I2S configuration for PCM5101
static const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,  // Minimal buffers
    .dma_buf_len = 64,   // Small buffer size
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
};

static const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
};

static bool audio_initialized = false;
static bool audio_enabled = true;
static int audio_volume = AUDIO_VOLUME_DEFAULT;
static sound_type_t timer_sound = SOUND_TIMER_FINISH;

// Sound definitions
typedef struct {
    int frequency;
    int duration_ms;
    int repeat_count;
    int repeat_delay_ms;
} sound_def_t;

static const sound_def_t sounds[SOUND_COUNT] = {
    {1000, 200, 2, 100},    // SOUND_TIMER_FINISH: 2x beep
    {800, 150, 3, 80},      // SOUND_TIMER_FINISH_ALT1: 3x beep
    {1200, 100, 1, 0},      // SOUND_TIMER_FINISH_ALT2: high beep
    {600, 300, 1, 0},       // SOUND_TIMER_FINISH_ALT3: long beep
    {800, 50, 1, 0},        // SOUND_BUTTON_CLICK: short click
    {1200, 100, 1, 0},      // SOUND_SUCCESS: high beep
    {400, 300, 1, 0},       // SOUND_ERROR: low beep
    {600, 150, 3, 80}       // SOUND_STARTUP: 3x startup beep
};

void simple_audio_init() {
    if (audio_initialized) return;
    
    // Load settings from NVS
    audio_enabled = player_store.getInt("audio_enabled", AUDIO_ENABLED_DEFAULT) == 1;
    audio_volume = player_store.getInt("audio_volume", AUDIO_VOLUME_DEFAULT);
    timer_sound = (sound_type_t)player_store.getInt("timer_sound", TIMER_SOUND_DEFAULT);
    
    if (!audio_enabled) {
        printf("[Audio] Audio disabled in settings\n");
        return;
    }
    
    esp_err_t ret = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (ret != ESP_OK) {
        printf("[Audio] I2S driver install failed: %s\n", esp_err_to_name(ret));
        audio_enabled = false;  // Disable on failure
        return;
    }
    
    ret = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (ret != ESP_OK) {
        printf("[Audio] I2S pin config failed: %s\n", esp_err_to_name(ret));
        audio_enabled = false;  // Disable on failure
        return;
    }
    
    audio_initialized = true;
    printf("[Audio] Simple I2S audio initialized (volume: %d)\n", audio_volume);
}

void simple_audio_beep(int frequency, int duration_ms) {
    if (!audio_enabled || !audio_initialized) {
        simple_audio_init();
        if (!audio_enabled || !audio_initialized) return;
    }
    
    int samples = (44100 * duration_ms) / 1000;
    int16_t *buffer = (int16_t*)malloc(samples * 2 * sizeof(int16_t));
    if (!buffer) {
        printf("[Audio] Failed to allocate buffer\n");
        return;
    }
    
    // Calculate volume (0-21 -> 0.0-1.0)
    float volume_factor = (float)audio_volume / AUDIO_VOLUME_MAX;
    
    // Generate sine wave
    for (int i = 0; i < samples; i++) {
        float sample = sin(2 * PI * frequency * i / 44100.0) * volume_factor;
        int16_t sample16 = (int16_t)(sample * 32767);
        buffer[i * 2] = sample16;     // Left channel
        buffer[i * 2 + 1] = sample16; // Right channel
    }
    
    size_t bytes_written;
    esp_err_t ret = i2s_write(I2S_NUM_0, buffer, samples * 2 * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    
    free(buffer);
    
    if (ret != ESP_OK) {
        printf("[Audio] I2S write failed: %s\n", esp_err_to_name(ret));
        // Disable audio on persistent failure
        audio_enabled = false;
        player_store.putInt("audio_enabled", 0);
    }
}

void simple_audio_play_sound(sound_type_t sound) {
    if (sound >= SOUND_COUNT) return;
    
    const sound_def_t *sound_def = &sounds[sound];
    
    for (int i = 0; i < sound_def->repeat_count; i++) {
        simple_audio_beep(sound_def->frequency, sound_def->duration_ms);
        if (i < sound_def->repeat_count - 1 && sound_def->repeat_delay_ms > 0) {
            delay(sound_def->repeat_delay_ms);
        }
    }
}

void simple_audio_set_volume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > AUDIO_VOLUME_MAX) volume = AUDIO_VOLUME_MAX;
    
    audio_volume = volume;
    player_store.putInt("audio_volume", volume);
    printf("[Audio] Volume set to %d\n", volume);
}

int simple_audio_get_volume() {
    return audio_volume;
}

void simple_audio_set_enabled(bool enabled) {
    audio_enabled = enabled;
    player_store.putInt("audio_enabled", enabled ? 1 : 0);
    
    if (enabled && !audio_initialized) {
        simple_audio_init();
    } else if (!enabled && audio_initialized) {
        simple_audio_cleanup();
    }
    
    printf("[Audio] Audio %s\n", enabled ? "enabled" : "disabled");
}

bool simple_audio_is_enabled() {
    return audio_enabled;
}

void simple_audio_set_timer_sound(sound_type_t sound) {
    if (sound >= SOUND_TIMER_FINISH && sound <= SOUND_TIMER_FINISH_ALT3) {
        timer_sound = sound;
        player_store.putInt("timer_sound", (int)sound);
        printf("[Audio] Timer sound set to %d\n", sound);
    }
}

sound_type_t simple_audio_get_timer_sound() {
    return timer_sound;
}

void simple_audio_cleanup() {
    if (audio_initialized) {
        i2s_driver_uninstall(I2S_NUM_0);
        audio_initialized = false;
        printf("[Audio] I2S audio cleaned up\n");
    }
}

// Wrapper functions for settings compatibility
bool getAudioEnabled() {
    return simple_audio_is_enabled();
}

void toggleAudioEnabled() {
    simple_audio_set_enabled(!simple_audio_is_enabled());
}

int getVolume() {
    return simple_audio_get_volume();
}

void setVolume(int volume) {
    simple_audio_set_volume(volume);
}

bool getSoundEffectsEnabled() {
    return simple_audio_is_enabled(); // For now, same as audio enabled
}

void toggleSoundEffectsEnabled() {
    toggleAudioEnabled(); // For now, same as audio toggle
}
