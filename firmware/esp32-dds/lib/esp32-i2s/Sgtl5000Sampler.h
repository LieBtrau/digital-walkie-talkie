#pragma once

#include "I2SSampler.h"
#include "SampleSource.h"

class Sgtl5000Sampler : public I2SSampler
{
public:
    Sgtl5000Sampler(i2s_port_t i2sPort, i2s_pin_config_t* pin_config);
    void start(SampleSink* sampleSink, QueueHandle_t xAudioSamplesQueue);

protected:
    void configureI2S();
};
