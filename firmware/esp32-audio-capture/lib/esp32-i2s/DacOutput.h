#pragma once
#include "I2SOutput.h"

class DacOutput : public I2SOutput
{
public:
    void start(SampleSource *sample_generator);
private:
};