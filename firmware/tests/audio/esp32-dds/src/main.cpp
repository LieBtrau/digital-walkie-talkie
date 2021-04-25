#include <Arduino.h>
#include "SinWaveGenerator.h"
#include "DacOutput.h"

// i2s pins
DacOutput *output;
SampleSource *sampleSource;

void setup()
{
  Serial.begin(115200);

  Serial.printf("Build %s\r\n", __TIMESTAMP__);
  Serial.printf("CPU clock speed: %uMHz\r\n",ESP.getCpuFreqMHz());

  sampleSource = new SinWaveGenerator(8000, 100, 255);

  Serial.println("Starting I2S Output");
  output = new DacOutput();
  output->start(sampleSource);
}

void loop()
{
  // nothing to do here - everything is taken care of by tasks
}