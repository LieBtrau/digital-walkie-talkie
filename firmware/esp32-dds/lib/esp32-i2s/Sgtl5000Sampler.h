#pragma once

#include "I2SSampler.h"

class Sgtl5000Sampler : public I2SSampler
{
private:
    i2s_port_t _i2s_port;
    i2s_pin_config_t m_pin_config;
    int _sampleRate = 8000; //8kHz

protected:
    void configureI2S();

public:
    Sgtl5000Sampler(i2s_port_t i2sPort, i2s_pin_config_t pin_config);
    void start(QueueHandle_t samplesQueue, int packetSize);

};
