//https://github.com/atomic14/esp32_audio/
#pragma once

#include "I2SSampler.h"

class I2SMEMSSampler : public I2SSampler
{
private:
    i2s_pin_config_t m_i2sPins;
    bool m_fixSPH0645;

protected:
    void configureI2S();

public:
    I2SMEMSSampler(i2s_pin_config_t* i2sPins, bool fixSPH0645 = false);
    void start(TaskHandle_t writerTaskHandle);
};
