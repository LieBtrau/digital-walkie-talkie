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
    // src of samples for us to play
    QueueHandle_t m_samplesQueue;
    int m_packetSize;
    TaskHandle_t m_i2s_writerTaskHandle = NULL;

protected:
    void start(i2s_config_t i2sConfig, QueueHandle_t samplesQueue);
    void startTask();
    virtual void configureI2S() = 0;
    i2s_port_t getI2SPort()
    {
        return m_i2sPort;
    }

public:
    void start(i2s_port_t i2sPort, i2s_config_t i2sConfig, QueueHandle_t samplesQueue, int pktSize);
    void stop();
    friend void i2sWriterTask(void *param);
};
