#include "BAT_Driver.h"
#include "driver/adc.h"

float BAT_analogVolts = 0;

void BAT_Init(void)
{
    // Configure ADC1 channel 7 (GPIO8) for 12-bit width and 11dB attenuation
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_12); // ADC_ATTEN_DB_11 is deprecated, use ADC_ATTEN_DB_12
}
float BAT_Get_Volts(void)
{
    int raw = adc1_get_raw(ADC1_CHANNEL_7);
    // Convert raw value to voltage (3.3V reference, 12-bit ADC)
    float voltage = ((float)raw / 4095.0f) * 3.3f;
    BAT_analogVolts = (voltage * 3.0f) / Measurement_offset;
    // printf("BAT voltage : %.2f V\r\n", BAT_analogVolts);
    return BAT_analogVolts;
}