//https://github.com/atomic14/esp32_audio/
#include <Arduino.h>
#include "driver/i2s.h"
#include "I2SOutput.h"
#include "SampleSource.h"

void i2sWriterTask(void *param)
{
    I2SOutput *output = (I2SOutput *)param;
    int availableBytes = 0;
    int buffer_position = 0;
    Frame_t frames[output->m_packetSize];
    while (true)
    {
        // wait for some data to be requested
        i2s_event_t evt;
        if (xQueueReceive(output->m_i2sEventQueue, &evt, portMAX_DELAY) == pdPASS)
        {
            if (evt.type == I2S_EVENT_TX_DONE)
            {
                size_t bytesWritten = 0;
                do
                {
                    if (availableBytes == 0 && xQueueReceive(output->m_samplesQueue, &frames, portMAX_DELAY) == pdTRUE)
                    {
                        // how many bytes do we now have to send
                        availableBytes = output->m_packetSize * sizeof(Frame_t);
                        // reset the buffer position back to the start
                        buffer_position = 0;
                    }
                    // do we have something to write?
                    if (availableBytes > 0)
                    {
                        // write data to the i2s peripheral
                        i2s_write(output->m_i2sPort, buffer_position + (uint8_t *)frames,
                                  availableBytes, &bytesWritten, portMAX_DELAY);
                        availableBytes -= bytesWritten;
                        buffer_position += bytesWritten;
                    }
               } while (bytesWritten > 0);
            }
        }
    }
}

void I2SOutput::start(i2s_port_t i2sPort, i2s_pin_config_t &i2sPins, i2s_config_t i2sConfig, QueueHandle_t samplesQueue, int pktSize)
{
    m_samplesQueue = samplesQueue;
    m_i2sPort = i2sPort;
    m_packetSize = pktSize;
    //install and start i2s driver
    i2s_driver_install(m_i2sPort, &i2sConfig, 4, &m_i2sEventQueue);
    // set up the i2s pins
    i2s_set_pin(m_i2sPort, &i2sPins);
    startTask();
}

//For internal DAC-only (GPIO 25 & GPIO 26)
void I2SOutput::start(i2s_config_t i2sConfig, QueueHandle_t samplesQueue)
{
    m_samplesQueue = samplesQueue;
    m_i2sPort = I2S_NUM_0;
    //install and start i2s driver
    i2s_driver_install(m_i2sPort, &i2sConfig, 4, &m_i2sEventQueue);
    // set up the i2s pins
    i2s_set_dac_mode(I2S_DAC_CHANNEL_LEFT_EN);
    startTask();
}

void I2SOutput::startTask()
{
    // clear the DMA buffers
    i2s_zero_dma_buffer(m_i2sPort);
    // start a task to write samples to the i2s peripheral
    TaskHandle_t writerTaskHandle;
    xTaskCreate(i2sWriterTask, "i2s Writer Task", 4096, this, 1, &writerTaskHandle);
}