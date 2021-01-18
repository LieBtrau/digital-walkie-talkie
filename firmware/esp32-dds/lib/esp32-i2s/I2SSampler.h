//https://github.com/atomic14/esp32_audio/
#pragma once

#include <Arduino.h>
#include "driver/i2s.h"

/**
 * Base Class for both the ADC and I2S sampler
 **/
class I2SSampler
{
private:
    // double buffer so we can be capturing samples while sending data
    uint16_t *m_i2s_in_Buffer1;
    uint16_t *m_i2s_in_Buffer2;
    // current position in the i2s_in buffer
    int32_t m_i2s_in_BufferPos = 0;
    // current i2s_in buffer
    uint16_t *m_current_i2s_in_Buffer;
    // buffer containing samples that have been captured already
    uint16_t *m_captured_i2s_in_Buffer;
    // size of the i2s_in_ buffer in samples
    int m_bufferSizeInSamples;
    // I2S reader task
    TaskHandle_t m_readerTaskHandle;
    // writer task
    TaskHandle_t m_writerTaskHandle;
    // i2s reader queue
    QueueHandle_t m_i2sEventQueue;
    // i2s port
    i2s_port_t m_i2sPort;
    void processI2SData(uint16_t *i2sData, size_t bytesRead);

protected:
    static const uint16_t I2S_IN_DMA_BUFFER_LEN = 256; //size in bytes
    void addSample(uint16_t sample);
    virtual void configureI2S() = 0;
    i2s_port_t getI2SPort()
    {
        return m_i2sPort;
    }

public:
    int32_t getBufferSizeInSamples()
    {
        return m_bufferSizeInSamples;
    };
    uint16_t* getCaptured_i2s_in_Buffer()
    {
        return m_captured_i2s_in_Buffer;
    }
    void start(i2s_port_t i2sPort, i2s_config_t &i2sConfig, int bufferSizeInSamples, TaskHandle_t writerTaskHandle);

    friend void i2sReaderTask(void *param);
};
