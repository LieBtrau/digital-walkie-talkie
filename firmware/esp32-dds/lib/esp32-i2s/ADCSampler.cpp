//https://github.com/atomic14/esp32_audio/
//https://github.com/rkinnett/ESP32-2-Way-Audio-Relay/blob/master/esp32_vban_2way_test/esp32_vban_2way_test.ino

#include <Arduino.h>
#include "driver/i2s.h"
#include "driver/adc.h"
#include "ADCSampler.h"
#include "esp_adc_cal.h"

ADCSampler::ADCSampler(adc_unit_t adcUnit, adc1_channel_t adcChannel)
{
    m_adcUnit = adcUnit;
    m_adcChannel = adcChannel;
}

void ADCSampler::configureI2S()
{
    // esp_adc_cal_characteristics_t *adc_chars;
    // const uint32_t DEFAULT_VREF = 1100; // calibration values for the adc
    //Range 0-4096
    adc1_config_width(ADC_WIDTH_BIT_12);
    // full voltage range
    adc1_config_channel_atten(m_adcChannel, ADC_ATTEN_DB_11);

    // // check to see what calibration is available
    // if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
    // {
    //     Serial.println("Using voltage ref stored in eFuse");
    // }
    // if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
    // {
    //     Serial.println("Using two point values from eFuse");
    // }
    // if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_DEFAULT_VREF) == ESP_OK)
    // {
    //     Serial.println("Using default VREF");
    // }
    // //Characterize ADC
    // adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    // esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

    //init ADC pad
    i2s_set_adc_mode(m_adcUnit, m_adcChannel);
    // enable the adc
    i2s_adc_enable(getI2SPort());
}

void ADCSampler::start(int32_t bufferSizeInSamples, TaskHandle_t writerTaskHandle)
{
    // i2s config for using the internal ADC
    i2s_config_t adcI2SConfig = {
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
    I2SSampler::start(I2S_NUM_0, adcI2SConfig, bufferSizeInSamples, writerTaskHandle);
}