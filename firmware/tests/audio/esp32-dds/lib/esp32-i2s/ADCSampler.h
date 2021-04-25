//https://github.com/atomic14/esp32_audio/
#pragma once

#include "I2SSampler.h"

class ADCSampler : public I2SSampler
{
private:
    adc_unit_t m_adcUnit;
    adc1_channel_t m_adcChannel;
    i2s_config_t adcI2SConfig;

protected:
    void configureI2S();

public:
    ADCSampler(adc_unit_t adc_unit, adc1_channel_t adc_channel);
    void start(QueueHandle_t samplesQueue);

};
