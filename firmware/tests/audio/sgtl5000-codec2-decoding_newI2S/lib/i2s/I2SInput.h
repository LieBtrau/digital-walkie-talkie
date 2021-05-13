//Roughly based on : https://github.com/atomic14/esp32_audio/
#pragma once

#include <Arduino.h>
#include "driver/i2s.h"
#include "SampleSink.h"

/**
 * Base Class for both the ADC and I2S sampler
 **/
class I2SInput
{
private:
    // i2s port
    i2s_port_t m_i2sPort;
    // i2s pin settings
    i2s_pin_config_t *m_pin_config;
    // I2S reader task
    TaskHandle_t m_i2s_readerTaskHandle = NULL;
    // consumer of the samples we provide
    SampleSink* m_sample_consumer;
    // Sample buffer
    int16_t *m_frames;
    // current position in the i2s_in buffer
    int32_t m_i2s_in_BufferPos = 0;

protected:
    static const uint16_t I2S_IN_DMA_BUFFER_LEN = 64; //size in bytes
    void start(i2s_config_t *i2sConfig, SampleSink *sample_consumer);
    void startTask();
    void addSample(int16_t sample);
    virtual void configureI2S() = 0;

public:
    I2SInput(i2s_port_t i2sPort, i2s_pin_config_t *pin_config) : m_i2sPort(i2sPort), m_pin_config(pin_config){};
    void stop();
    friend void i2sReaderTask(void *param);
};
