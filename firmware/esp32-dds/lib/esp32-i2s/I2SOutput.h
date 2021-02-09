//https://github.com/atomic14/esp32_audio/
#pragma once

#include "Arduino.h"
#include "driver/i2s.h"

/**
 * Base Class for both the DAC and other outputs
 **/
class I2SOutput
{
private:
    // i2s port
    i2s_port_t m_i2sPort;
    // i2s pin settings
    i2s_pin_config_t* m_pin_config;
    // src of samples for us to play
    QueueHandle_t m_samplesQueue;
    // each item in the queue contains this number of audio samples
    int m_sampleCount;
    // handle to the task that will write the samples to the I2S peripheral
    TaskHandle_t m_i2s_writerTaskHandle = NULL;

protected:
    void start(i2s_config_t* i2sConfig, QueueHandle_t samplesQueue, int pktSize);
    void startTask();
    virtual void configureI2S() = 0;

public:
    I2SOutput(i2s_port_t i2sPort, i2s_pin_config_t* pin_config): m_i2sPort(i2sPort), m_pin_config(pin_config){}
    void stop();
    friend void i2sWriterTask(void *param);
};
