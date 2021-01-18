#pragma once
#include "I2SOutput.h"
#include "SampleSource.h"

class Sgtl5000_Output : public I2SOutput
{
public:
    Sgtl5000_Output(byte pin_SCK, byte pin_WS, byte pin_DOUT);
    void start(SampleSource *sample_generator, QueueHandle_t xQueue);
private:
    byte _pin_SCK;
    byte _pin_WS;
    byte _pin_DOUT;
};