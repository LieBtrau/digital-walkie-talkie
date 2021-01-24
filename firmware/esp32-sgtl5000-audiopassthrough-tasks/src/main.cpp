/**
 * Output decoded codec2 packets to SGTL5000 headphone and line out.
 * 
 * Maybe helpful for debugging:
 *  http://www.iotsharing.com/2017/07/how-to-use-arduino-esp32-i2s-to-play-wav-music-from-sdcard.html
 *  https://diyi0t.com/i2s-sound-tutorial-for-esp32/
 *  https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/i2s.html
 * 
 * Signal name                              Adafruit 1780                   NodeMCU-32S
 * 3V3                                      3.3V                            3.3V
 * GND                                      GND                             GND
 *                                          G                               both ground pins of the Adafruit 1780 must be connected!
 * MCLK (Audio Master Clock)                23 (MCLK)                       GPIO0 (or another CLK_OUT pin)
 *      Sinusoidal signal, 1.8Vpp, 1.7Vavg.
 * LRCLK                                    20 (LRCLK)                      GPIO25 (or other GPIO)
 *      Audio Left/Right Clock
 *      WS (Word Select)
 *      48kHz square wave
 * BCLK                                     21 (BCLK)                       GPIO26 (or other GPIO)
 *      SCLK (Audio Bit Clock)
 *      1.54MHz
 * DOUT                                     8                               GPIO33
 *      Audio Data from Audio Shield to Teensy
 * DIN                                      7                               GPIO23 (or other GPIO)
 *      Audio Data from MCU to Audio Shield
 * SDA                                      18                              GPIO21
 * SCL                                      19                              GPIO22
 */

#include "Sgtl5000Sampler.h"
#include "Sgtl5000_Output.h"
#include "control_sgtl5000.h"

//Sgtl5000_Output *output;
I2SSampler *sampler;
AudioControlSGTL5000 audioShield;
QueueHandle_t xQueue;
int sampleCtr = 0;
const int BUFSIZE = 330;
int16_t sampleBuffer[BUFSIZE];
bool ready = false;

void vCaptureTask(void *pvParameters)
{
	int16_t samples[I2SSampler::FRAMESIZE];

	for (;;)
	{
		while (xQueueReceive(xQueue, samples, portMAX_DELAY) == pdTRUE)
		{
			if (sampleCtr + I2SSampler::FRAMESIZE < BUFSIZE)
			{
				for (int i = 0; i < I2SSampler::FRAMESIZE; i++)
				{
					sampleBuffer[sampleCtr++] = samples[i];
					//Serial.printf("%d, %d\n", sampleCtr - 1, sampleBuffer[sampleCtr - 1]);
					//Serial.printf("\t%d, %d\n", i, samples[i]);
				}
			}
			else
			{
				ready = true;
			}
		}
		taskYIELD();
	}
}

void setup()
{
	Serial.begin(115200);

	Serial.printf("Build %s\r\n", __TIMESTAMP__);
	Serial.printf("CPU clock speed: %uMHz\r\n", ESP.getCpuFreqMHz());

	sampler = new Sgtl5000Sampler(I2S_NUM_0, 26, 25, 33);
	xQueue = xQueueCreate(3, sizeof(int16_t) * I2SSampler::FRAMESIZE);
	if (xQueue == NULL)
	{
		Serial.println("Can't create queue");
		while (true)
			;
	}
	((Sgtl5000Sampler *)sampler)->start(xQueue);

	Serial.printf("SGTL5000 %s initialized.\n", audioShield.enable() ? "is" : "not");
	xTaskCreate(vCaptureTask, "Capture1", 4096, NULL, 2, NULL);
}

void loop()
{
	// nothing to do here - everything is taken care of by tasks
	if (ready)
	{
		for (int i = 0; i < sampleCtr; i++)
		{
			Serial.printf("%d, %d\n", i, sampleBuffer[i]);
		}
		sampleCtr=0;
		ready=false;
		delay(2000);
	}
}