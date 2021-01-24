#pragma once

#include "I2SSampler.h"

class Sgtl5000Sampler : public I2SSampler
{
private:
    i2s_port_t _i2s_port;
    byte _pin_SCK;
    byte _pin_WS;
    byte _pin_DIN;
    int _sampleRate = 8000; //8kHz

protected:
    void configureI2S();

public:
    Sgtl5000Sampler(i2s_port_t i2sPort, byte pin_SCK, byte pin_WS, byte pin_DIN);
    void start(QueueHandle_t samplesQueue);

};
