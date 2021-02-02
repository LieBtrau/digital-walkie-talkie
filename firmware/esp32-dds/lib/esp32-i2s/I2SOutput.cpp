//https://github.com/atomic14/esp32_audio/
#include <Arduino.h>
#include "driver/i2s.h"
#include "I2SOutput.h"
#include "SampleSource.h"

bool writing=false;
bool wanttoStop=false;

void i2sWriterTask(void *param)
{
    I2SOutput *output = (I2SOutput *)param;
    int availableBytes = 0;
    int buffer_position = 0;
    Frame_t frames[output->m_packetSize];
        size_t bytesWritten = 0;
    for (;;)
    {
        // wait for some data to be requested
        //i2s_event_t evt;
        // if (xQueueReceive(output->m_i2sEventQueue, &evt, portMAX_DELAY) == pdPASS)
        // {
        // if (evt.type == I2S_EVENT_TX_DONE)
        // {
        // do
        // {
            Serial.print("a");
            if (!wanttoStop && availableBytes == 0 && xQueueReceive(output->m_samplesQueue, frames, portMAX_DELAY) == pdTRUE)
            {
                Serial.print("b");
                // how many bytes do we now have to send
                availableBytes = output->m_packetSize * sizeof(Frame_t);
                // reset the buffer position back to the start
                buffer_position = 0;
            }
            // do we have something to write?
            if (availableBytes > 0)
            {
                Serial.print("c");
                // write data to the i2s peripheral
                writing=true;
                ESP_ERROR_CHECK(i2s_write(output->m_i2sPort, buffer_position + (uint8_t *)frames,
                                          availableBytes, &bytesWritten, portMAX_DELAY));
                                          Serial.print("d");
                availableBytes -= bytesWritten;
                buffer_position += bytesWritten;
                writing=false;
            }
        // } while (bytesWritten > 0);
        Serial.print("e");
        taskYIELD();
        //            }
        // }
    }
}

void I2SOutput::start(i2s_port_t i2sPort, i2s_config_t i2sConfig, QueueHandle_t samplesQueue, int pktSize)
{
    m_i2sPort = i2sPort;
    m_samplesQueue = samplesQueue;
    //xQueueReset(m_samplesQueue);
    m_packetSize = pktSize;
    //install and start i2s driver
    ESP_ERROR_CHECK(i2s_driver_install(m_i2sPort, &i2sConfig, 0, NULL)); //4, &m_i2sEventQueue));
    // set up the I2S configuration from the subclass
    startTask();
}

//For internal DAC-only (GPIO 25 & GPIO 26)
void I2SOutput::start(i2s_config_t i2sConfig, QueueHandle_t samplesQueue)
{
    m_samplesQueue = samplesQueue;
    //xQueueReset(m_samplesQueue);
    m_i2sPort = I2S_NUM_0;
    //install and start i2s driver
    ESP_ERROR_CHECK(i2s_driver_install(m_i2sPort, &i2sConfig, 0, NULL)); //4, &m_i2sEventQueue));
    // set up the i2s pins
    ESP_ERROR_CHECK(i2s_set_dac_mode(I2S_DAC_CHANNEL_LEFT_EN));
    startTask();
}

void I2SOutput::startTask()
{
    // clear the DMA buffers
    ESP_ERROR_CHECK(i2s_zero_dma_buffer(m_i2sPort));
    // start a task to write samples to the i2s peripheral
    if (m_i2s_writerTaskHandle == NULL)
    {
        configureI2S();
        xTaskCreate(i2sWriterTask, "i2s Writer Task", 4096, this, 1, &m_i2s_writerTaskHandle);
    }
    vTaskDelay(1);
    Serial.println("task started");
    wanttoStop=false;
}

void I2SOutput::stop()
{
    wanttoStop=true;
    while(writing)vTaskDelay(1);;
    ESP_ERROR_CHECK(i2s_stop(m_i2sPort));
    ESP_ERROR_CHECK(i2s_zero_dma_buffer(m_i2sPort));
    ESP_ERROR_CHECK(i2s_driver_uninstall(m_i2sPort));
    //xQueueReset(m_i2sEventQueue);
}