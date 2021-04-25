#include <Arduino.h>
#include "I2SSampler.h"
#include "ADCSampler.h"

ADCSampler *adcSampler;

// Task to write samples from ADC to our server
void adcWriterTask(void *param)
{
  I2SSampler *sampler = (I2SSampler *)param;
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
  while (true)
  {
    // wait for some samples to save
    uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    if (ulNotificationValue > 0)
    {
      uint16_t *samples = sampler->getCaptured_i2s_in_Buffer();
      int sampleCount = sampler->getBufferSizeInSamples();
      for(int i=0;i<sampleCount;i++)
      {
        Serial.printf("%u ", samples[i]);
      }
      Serial.println();
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Started up");

  //   input from analog microphones such as the MAX9814 or MAX4466
  //   internal analog to digital converter sampling using i2s
  //   create our samplers
  adcSampler = new ADCSampler(ADC_UNIT_1, ADC1_CHANNEL_7);

  //set up the adc sample writer task
  TaskHandle_t adcWriterTaskHandle;
  xTaskCreatePinnedToCore(adcWriterTask, "ADC Writer Task", 4096, adcSampler, 1, &adcWriterTaskHandle, 1);
  adcSampler->start(50, adcWriterTaskHandle);
}

void loop()
{
}