//https://github.com/atomic14/esp32_audio/
#include <Arduino.h>
#include "I2SSampler.h"
#include "driver/i2s.h"

void i2sReaderTask(void *param);

void I2SSampler::start(i2s_port_t i2sPort, i2s_config_t &i2sConfig, QueueHandle_t samplesQueue, int pktSize)
{
    m_i2sPort = i2sPort;
    m_samplesQueue = samplesQueue;
    m_packetSize = pktSize;
    m_frames = (int16_t *)calloc(m_packetSize, sizeof(int16_t));
    //install and start i2s driver
    ESP_ERROR_CHECK(i2s_driver_install(m_i2sPort, &i2sConfig, 0, NULL)); //4, &m_i2sEventQueue));
    // set up the I2S configuration from the subclass
    configureI2S();
    // clear the DMA buffers
    ESP_ERROR_CHECK(i2s_zero_dma_buffer(m_i2sPort));
    // start a task to read samples from the ADC
    TaskHandle_t readerTaskHandle;
    xTaskCreate(i2sReaderTask, "i2s Reader Task", 4096, this, 1, &readerTaskHandle);
}

void I2SSampler::stop()
{
    //wanttoStop = true;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    ESP_ERROR_CHECK(i2s_stop(m_i2sPort));
    ESP_ERROR_CHECK(i2s_zero_dma_buffer(m_i2sPort));
    ESP_ERROR_CHECK(i2s_driver_uninstall(m_i2sPort));
}

void I2SSampler::addSample(int16_t sample)
{
    // add the sample to the current i2s_in_ buffer
    m_frames[m_i2s_in_BufferPos++] = sample;
    // have we filled the buffer with data?
    if (m_i2s_in_BufferPos == m_packetSize)
    {
        //reset the buffer position
        m_i2s_in_BufferPos = 0;
        // add the frameBuffer to the queue
        xQueueSendToBack(m_samplesQueue, m_frames, portMAX_DELAY);
    }
}

void i2sReaderTask(void *param)
{
    I2SSampler *sampler = (I2SSampler *)param;
    int16_t i2sData[I2SSampler::I2S_IN_DMA_BUFFER_LEN >> 1];
    size_t bytesRead = 0;
    for (;;)
    {
        if (i2s_read(sampler->getI2SPort(), i2sData, sizeof(i2sData), &bytesRead, portMAX_DELAY) == ESP_OK)
        {
            // process the raw data
            //https://github.com/rkinnett/ESP32-2-Way-Audio-Relay/blob/master/esp32_vban_2way_test/esp32_vban_2way_test.ino
            //// Per esp32.com forum topic 11023, esp32 swaps even/odd samples,
            ////   i.g. samples (0 1) (2 3) (4 5) are stored as (1 0) (3 2) (5 4) ..
            ////   Have to deinterleave manually; use xor "^1" to leap frog indices
            for (int i = 0; i < (bytesRead >> 1); i++)
            {
                sampler->addSample(i2sData[i ^ 1]);
            }
        }
        taskYIELD();
    }
}


