//https://github.com/atomic14/esp32_audio/
#include <Arduino.h>
#include "I2SSampler.h"
#include "driver/i2s.h"

void i2sReaderTask(void *param);

void I2SSampler::start(i2s_port_t i2sPort, i2s_config_t &i2sConfig, int bufferSizeInSamples, TaskHandle_t writerTaskHandle)
{
    m_i2sPort = i2sPort;
    m_writerTaskHandle = writerTaskHandle;
    m_bufferSizeInSamples = bufferSizeInSamples;
    m_i2s_in_Buffer1 = (uint16_t*)calloc(bufferSizeInSamples, sizeof(uint16_t));
    m_i2s_in_Buffer2 = (uint16_t*)calloc(bufferSizeInSamples, sizeof(uint16_t));

    m_current_i2s_in_Buffer = m_i2s_in_Buffer1;
    m_captured_i2s_in_Buffer = m_i2s_in_Buffer2;

    m_writerTaskHandle = writerTaskHandle;
    //install and start i2s driver
    i2s_driver_install(m_i2sPort, &i2sConfig, 4, &m_i2sQueue);
    // set up the I2S configuration from the subclass
    configureI2S();
    // clear the DMA buffers
    i2s_zero_dma_buffer(m_i2sPort);
    // start a task to read samples from the ADC
    TaskHandle_t readerTaskHandle;
    xTaskCreatePinnedToCore(i2sReaderTask, "i2s Reader Task", 4096, this, 1, &readerTaskHandle, 0);
}

void i2sReaderTask(void *param)
{
    I2SSampler *sampler = (I2SSampler *)param;
    while (true)
    {
        // wait for some data to arrive on the queue
        i2s_event_t evt;
        if (xQueueReceive(sampler->m_i2sQueue, &evt, portMAX_DELAY) == pdPASS)
        {
            if (evt.type == I2S_EVENT_RX_DONE)
            {
                size_t bytesRead = 0;
                do
                {
                    // read data from the I2S peripheral
                    uint16_t i2sData[I2SSampler::I2S_IN_DMA_BUFFER_LEN];
                    // read from i2s
                    // 0xFFF (4095)-> <= 0.128V
                    // 0xBCF (3023)-> =  1.00V
                    // 0x706 (1798)-> =  2.00V
                    // 0x12C (300)-> =  3.00V
                    // 0x000 (0) -> >= 3.15V
                    i2s_read(sampler->getI2SPort(), i2sData, sizeof(i2sData), &bytesRead, portMAX_DELAY);
                    // process the raw data
                    sampler->processI2SData(i2sData, bytesRead);
                } while (bytesRead > 0);
            }
        }
    }
}

/**
 * Process the raw data that have been read from the I2S peripherals into samples
 **/
void I2SSampler::processI2SData(uint16_t *i2sData, size_t bytesRead)
{
    //// Per esp32.com forum topic 11023, esp32 swaps even/odd samples,
    ////   i.g. samples 0 1 2 3 4 5 are stored as 1 0 3 2 5 4 ..
    ////   Have to deinterleave manually; use xor "^1" to leap frog indices
    //// Also need to mask upper 4 bits which contain channel info (see gitter chat between me-no-dev and bzeeman)
    for (int i = 0; i < (bytesRead >> 1); i++)
    {
        uint16_t adcSample = 0xFFF - (i2sData[i ^ 1] & 0xFFF);
        //Serial.printf("%u ", adcSample);
        addSample(adcSample);
    }
    Serial.println();
}

void I2SSampler::addSample(uint16_t sample)
{
    // add the sample to the current i2s_in_ buffer
    m_current_i2s_in_Buffer[m_i2s_in_BufferPos++] = sample;
    // have we filled the buffer with data?
    if (m_i2s_in_BufferPos == m_bufferSizeInSamples)
    {
        // swap to the other buffer
        std::swap(m_current_i2s_in_Buffer, m_captured_i2s_in_Buffer);
        // reset the buffer position
        m_i2s_in_BufferPos = 0;
        // tell the writer task to save the data
        xTaskNotify(m_writerTaskHandle, 1, eIncrement);
    }
}

