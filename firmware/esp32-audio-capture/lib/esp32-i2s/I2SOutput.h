//https://github.com/atomic14/esp32_audio/
#pragma once

#include <Arduino.h>
#include "driver/i2s.h"

class SampleSource;

/**
 * Base Class for both the DAC and other outputs
 **/
class I2SOutput
{
private:
    // I2S write task
    TaskHandle_t m_i2sWriterTaskHandle;
    // i2s writer queue
    QueueHandle_t m_i2sQueue;
    // i2s port
    i2s_port_t m_i2sPort;
    // src of samples for us to play
    SampleSource *m_sample_generator;
protected:
    void start(i2s_config_t i2sConfig, SampleSource *sample_generator);
    void startTask();

public:
    void start(i2s_port_t i2sPort, i2s_pin_config_t &i2sPins, i2s_config_t i2sConfig, SampleSource *sample_generator);
    friend void i2sWriterTask(void *param);
};
