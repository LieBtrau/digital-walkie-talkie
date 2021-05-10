
#include <Arduino.h>
#include "driver/i2s.h"
#include "SampleSource.h"
#include "I2SOutput.h"

bool wanttoStopEncoding = false;
static TaskHandle_t xTaskToNotify = NULL;

void i2sWriterTask(void *param)
{
    I2SOutput *output = (I2SOutput *)param;
    int availableBytes = 0;
    int buffer_position = 0;
    int sampleCount = output->m_sample_generator->getFrameSampleCount();
    Frame_t frames[sampleCount];
    size_t bytesWritten = 0;
    for (;;)
    {
        if (wanttoStopEncoding)
        {
            xTaskNotifyGive(xTaskToNotify);
            vTaskSuspend(NULL);
        }
        else
        {
            if (availableBytes == 0)
            {
                // get some frames from the wave file - a frame consists of a 16 bit left and right sample
                output->m_sample_generator->getFrames(frames, sampleCount);
                // how many bytes do we now have to send
                availableBytes = sampleCount * sizeof(Frame_t);
                // reset the buffer position back to the start
                buffer_position = 0;
            }
            // do we have something to write?
            if (availableBytes > 0)
            {
                // write data to the i2s peripheral
                ESP_ERROR_CHECK(i2s_write(output->m_i2sPort, buffer_position + (uint8_t *)frames, availableBytes, &bytesWritten, portMAX_DELAY));
                availableBytes -= bytesWritten;
                buffer_position += bytesWritten;
            }
        }
        taskYIELD();
    }
}

void I2SOutput::start(i2s_config_t *i2sConfig, SampleSource *sample_generator)
{
    m_sample_generator = sample_generator;
    //install and start i2s driver
    ESP_ERROR_CHECK(i2s_driver_install(m_i2sPort, i2sConfig, 0, NULL));
    // set up the I2S configuration from the subclass
    startTask();
}

void I2SOutput::startTask()
{
    // clear the DMA buffers
    ESP_ERROR_CHECK(i2s_zero_dma_buffer(m_i2sPort));
    ESP_ERROR_CHECK(i2s_set_pin(m_i2sPort, m_pin_config));
    configureI2S();
    if (m_i2s_writerTaskHandle == NULL)
    {
        xTaskCreate(i2sWriterTask, "i2s Writer Task", 4096, this, 1, &m_i2s_writerTaskHandle);
    }
    vTaskDelay(1);
    vTaskResume(m_i2s_writerTaskHandle);
    xTaskToNotify = xTaskGetCurrentTaskHandle();
    wanttoStopEncoding = false;
}

void I2SOutput::stop()
{
    wanttoStopEncoding = true;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    ESP_ERROR_CHECK(i2s_stop(m_i2sPort));
    ESP_ERROR_CHECK(i2s_zero_dma_buffer(m_i2sPort));
    ESP_ERROR_CHECK(i2s_driver_uninstall(m_i2sPort));
}