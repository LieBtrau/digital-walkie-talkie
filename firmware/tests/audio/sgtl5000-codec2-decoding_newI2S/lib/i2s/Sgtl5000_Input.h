#pragma once

#include "I2SInput.h"

class Sgtl5000_Input : public I2SInput
{
public:
    Sgtl5000_Input(i2s_port_t i2sPort, i2s_pin_config_t *pin_config);
    void start(SampleSink *sampleSink);

protected:
    void configureI2S();

private:
    i2s_pin_config_t m_pin_config;
};
