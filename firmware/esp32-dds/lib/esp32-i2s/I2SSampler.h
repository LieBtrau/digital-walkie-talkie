//https://github.com/atomic14/esp32_audio/
#pragma once

#include <Arduino.h>
#include "driver/i2s.h"

/**
 * Base Class for both the ADC and I2S sampler
 **/
class I2SSampler
{
public:
    void start(i2s_port_t i2sPort, i2s_config_t &i2sConfig, QueueHandle_t samplesQueue, int pktSize);
    void stop();
    friend void i2sReaderTask(void *param);

protected:
    static const uint16_t I2S_IN_DMA_BUFFER_LEN = 64; //size in bytes
    void addSample(int16_t sample);
    virtual void configureI2S() = 0;
    i2s_port_t getI2SPort()
    {
        return m_i2sPort;
    }

private:
    // I2S reader task
    TaskHandle_t m_i2s_readerTaskHandle = NULL;
    // queue that will hold frames of captured samples
    QueueHandle_t m_samplesQueue;
    // the number of samples in one frame
    int m_packetSize;
    // i2s reader queue
    //QueueHandle_t m_i2sEventQueue;
    // i2s port
    i2s_port_t m_i2sPort;
    // Sample buffer
    int16_t* m_frames;
    // current position in the i2s_in buffer
    int32_t m_i2s_in_BufferPos = 0;

};
