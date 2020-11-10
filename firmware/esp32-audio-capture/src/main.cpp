#include <Arduino.h>
#include "SinWaveGenerator.h"
#include "DacOutput.h"

// i2s pins
DacOutput *output;
SampleSource *sampleSource;

void setup()
{
  Serial.begin(115200);

  Serial.println("Starting up");

  Serial.println("Created sample source");

  sampleSource = new SinWaveGenerator(40000, 10000, 0.75);

  Serial.println("Starting I2S Output");
  output = new DacOutput();
  output->start(sampleSource);
}

void loop()
{
  // nothing to do here - everything is taken care of by tasks
}