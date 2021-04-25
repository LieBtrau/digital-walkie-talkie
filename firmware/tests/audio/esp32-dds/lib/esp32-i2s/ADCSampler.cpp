//https://github.com/atomic14/esp32_audio/
//https://github.com/rkinnett/ESP32-2-Way-Audio-Relay/blob/master/esp32_vban_2way_test/esp32_vban_2way_test.ino

#include <Arduino.h>
#include "driver/i2s.h"
#include "driver/adc.h"
#include "ADCSampler.h"
#include "esp_adc_cal.h"

ADCSampler::ADCSampler(adc_unit_t adcUnit, adc1_channel_t adcChannel): I2SSampler(I2S_NUM_0, nullptr)
{
    m_adcUnit = adcUnit;
    m_adcChannel = adcChannel;
}

void ADCSampler::configureI2S()
{
    esp_adc_cal_characteristics_t *adc_chars;
    const uint32_t DEFAULT_VREF = 1100; // calibration values for the adc
    
    //Range 0-4096
    switch (m_adcUnit)
    {
    case ADC_UNIT_1:
        adc1_config_width(ADC_WIDTH_BIT_12);
        // full voltage range
        adc1_config_channel_atten(m_adcChannel, ADC_ATTEN_DB_11);
        break;
    default:
        break;
    }

    //Characterize ADC
    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(m_adcUnit, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
    {
        Serial.printf("eFuse Vref");
    }
    else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
    {
        Serial.printf("Two Point");
    }
    else
    {
        Serial.printf("Default");
    }

    //init ADC pad
    i2s_set_adc_mode(m_adcUnit, m_adcChannel);
    // enable the adc
    i2s_adc_enable(I2S_NUM_0);
}

void ADCSampler::start(QueueHandle_t samplesQueue)
{
    // i2s config for using the internal ADC
    adcI2SConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
        .sample_rate = 8000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = I2S_IN_DMA_BUFFER_LEN,
        .use_apll = false,
        /*.tx_desc_auto_clear = false,
        .fixed_mclk = 0*/
    };
    //Only works on I2S port 0
    I2SSampler::start(&adcI2SConfig, samplesQueue, 100);
}

    //// Also need to mask upper 4 bits which contain channel info (see gitter chat between me-no-dev and bzeeman)
