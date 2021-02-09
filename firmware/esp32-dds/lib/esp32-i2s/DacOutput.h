#pragma once
#include "I2SOutput.h"
#include "SampleSource.h"

class DacOutput : public I2SOutput
{
public:
    void start(SampleSource* sample_generator, QueueHandle_t xQueue);
private:
    i2s_config_t i2sConfig;
};