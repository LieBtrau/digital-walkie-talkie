//https://github.com/atomic14/esp32_audio/
#include <Arduino.h>
#include "I2SInput.h"
#include "driver/i2s.h"

static bool wanttoStopSampling = false;
static TaskHandle_t xTaskToNotify = NULL;

void i2sReaderTask(void *param)
{
    I2SInput *sampler = (I2SInput *)param;
    int16_t i2sData[I2SInput::I2S_IN_DMA_BUFFER_LEN >> 1];
    size_t bytesRead = 0;
    for (;;)
    {
        if (wanttoStopSampling)
        {
            xTaskNotifyGive(xTaskToNotify);
            vTaskSuspend(NULL);
        }
        else
        {
            if (i2s_read(sampler->m_i2sPort, i2sData, sizeof(i2sData), &bytesRead, portMAX_DELAY) == ESP_OK)
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
        }
        taskYIELD();
    }
}

void I2SInput::start(i2s_config_t *i2sConfig, SampleSink *sample_consumer)
{
    m_sample_consumer = sample_consumer;
    //install and start i2s driver
    ESP_ERROR_CHECK(i2s_driver_install(m_i2sPort, i2sConfig, 0, NULL));
    // set up the I2S configuration from the subclass
    startTask();
}

void I2SInput::startTask()
{
    // clear the DMA buffers
    ESP_ERROR_CHECK(i2s_zero_dma_buffer(m_i2sPort));
    ESP_ERROR_CHECK(i2s_set_pin(m_i2sPort, m_pin_config));
    configureI2S();
    if (m_i2s_readerTaskHandle == NULL)
    {
        m_frames = (int16_t *)calloc(m_sample_consumer->getFrameSampleCount(), sizeof(int16_t));
        // set up the I2S configuration from the subclass
        xTaskCreate(i2sReaderTask, "i2s Reader Task", 4096, this, 1, &m_i2s_readerTaskHandle);
    }
    vTaskDelay(1);
    vTaskResume(m_i2s_readerTaskHandle);
    xTaskToNotify = xTaskGetCurrentTaskHandle();
    wanttoStopSampling = false;
}

void I2SInput::stop()
{
    wanttoStopSampling = true;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    ESP_ERROR_CHECK(i2s_stop(m_i2sPort));
    ESP_ERROR_CHECK(i2s_zero_dma_buffer(m_i2sPort));
    ESP_ERROR_CHECK(i2s_driver_uninstall(m_i2sPort));
}

void I2SInput::addSample(int16_t sample)
{
    // add the sample to the current i2s_in_ buffer
    m_frames[m_i2s_in_BufferPos++] = sample;
    // have we filled the buffer with data?
    if (m_i2s_in_BufferPos == m_sample_consumer->getFrameSampleCount())
    {
        //reset the buffer position
        m_i2s_in_BufferPos = 0;
        // add the frameBuffer to the queue
        m_sample_consumer->setFrames(m_frames, m_sample_consumer->getFrameSampleCount());
    }
}
