#pragma once

#include <Arduino.h>
#include "driver/i2s.h"
#include "SampleSource.h"

/**
 * Base Class for both the ADC and I2S sampler
 **/
class I2SOutput
{
private:
    // i2s port
    i2s_port_t m_i2sPort;
    // i2s pin settings
    i2s_pin_config_t *m_pin_config;
    // I2S write task
    TaskHandle_t m_i2s_writerTaskHandle = NULL;
    // src of samples for us to play
    SampleSource *m_sample_generator;

protected:
    void start(i2s_config_t *i2sConfig, SampleSource *sample_generator);
    void startTask();
    virtual void configureI2S() = 0;

public:
    I2SOutput(i2s_port_t i2sPort, i2s_pin_config_t *pin_config) : m_i2sPort(i2sPort), m_pin_config(pin_config) {}
    void stop();
    friend void i2sWriterTask(void *param);
};