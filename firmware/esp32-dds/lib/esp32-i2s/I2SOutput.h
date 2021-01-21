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
    // I2S write task
    TaskHandle_t m_i2sWriterTaskHandle;
    // i2s writer queue
    QueueHandle_t m_i2sEventQueue;
    // i2s port
    i2s_port_t m_i2sPort;
    // src of samples for us to play
    QueueHandle_t m_samplesQueue;
    int m_packetSize;
protected:
    void start(i2s_config_t i2sConfig, QueueHandle_t samplesQueue);
    void startTask();

public:
    void start(i2s_port_t i2sPort, i2s_pin_config_t &i2sPins, i2s_config_t i2sConfig, QueueHandle_t samplesQueue, int pktSize);
    friend void i2sWriterTask(void *param);
};
