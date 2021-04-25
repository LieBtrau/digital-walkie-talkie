#pragma once
#include "I2SOutput.h"
#include "SampleSource.h"

class Sgtl5000_Output : public I2SOutput
{
public:
    Sgtl5000_Output(i2s_port_t i2sPort, i2s_pin_config_t* pin_config);
    void start(SampleSource *sample_generator, QueueHandle_t xAudioSamplesQueue);

protected:
    void configureI2S();

private:
    i2s_pin_config_t m_pin_config;
};