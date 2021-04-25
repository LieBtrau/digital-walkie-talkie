#include <Arduino.h>
#include "Codec2Generator.h"
#include "DacOutput.h"
#include "codec2.h"
#include "lookdave.h"

CODEC2 *codec2;
DacOutput *output;
SampleSource *sampleSource;

void setup()
{
  Serial.begin(115200);

  Serial.printf("Build %s\r\n", __TIMESTAMP__);
  Serial.printf("CPU clock speed: %uMHz\r\n",ESP.getCpuFreqMHz());

  codec2 = codec2_create(CODEC2_MODE_1200);
  codec2_set_natural_or_gray(codec2, 0);
  sampleSource = new Codec2Generator(codec2, lookdave_bit, 1);

//  codec2_destroy(codec2);

  Serial.println("Starting I2S Output");
  output = new DacOutput();
  output->start(sampleSource);
}

void loop()
{
  // nothing to do here - everything is taken care of by tasks
}
