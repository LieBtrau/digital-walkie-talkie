/**
 * Output decoded codec2 packets to SGTL5000 headphone and line out.
 * 
 * The on-off functionality in the audio stream simulates the PTT-behaviour.
 * 
 * A trade-off must be made between fast response on PTT-release (start I2S-RX task) and buffering.
 * Upon stopping the I2S-TX task (push PTT), audio stops immediately.
 * 
 * Remember that one codec2 1200bps packets contains 640bytes.
 * 
 * DMA-buf count		DMA-buf len			Delay from start output to I2S out (includes codec2 encoding)
 * 2					256					64ms
 * 2					1024				256ms
 * 4					1024				511ms
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
 * DOUT                                     8                               choose
 *      Audio Data from Audio Shield to Teensy
 * DIN                                      7                               GPIO23 (or other GPIO)
 *      Audio Data from MCU to Audio Shield
 * SDA                                      18                              GPIO21
 * SCL                                      19                              GPIO22
 */

#include "Codec2Decoder.h"
#include "Codec2Encoder.h"
#include "DacOutput.h"
#include "codec2.h"
#include "lookdave.h"
#include "Sgtl5000_Output.h"
#include "Sgtl5000Sampler.h"
#include "control_sgtl5000.h"
#include "AsyncDelay.h"
#include "base64.hpp"

CODEC2 *codec2;
Sgtl5000_Output *output;
Sgtl5000Sampler *input;
SampleSource *sampleSource;
SampleSink *sampleSink;
AudioControlSGTL5000 audioShield;
QueueHandle_t xAudioSamplesQueue;
QueueHandle_t xCodec2BitsQueue;
AsyncDelay delay_1s;
bool isPlaying = true;
const int iopin = 4; //maximum 3.3MHz digitalWrite toggle frequency.
int nbyte = 0;
int nbit_ctr = 0;
SemaphoreHandle_t xSemaphoreCodec2 = NULL;
unsigned char *bits;
int lineLength = 0;
static i2s_pin_config_t i2s_pin_config = {
	.bck_io_num = 26,	// Serial Clock (SCK)
	.ws_io_num = 25,	// Word Select (WS)
	.data_out_num = 23, // data out to audio codec
	.data_in_num = 33	// data from audio codec
};

void vEncoderTask(void *pvParameters)
{
	int nbyte = (codec2_bits_per_frame(codec2) + 7) / 8;
	unsigned char *bits = (unsigned char *)calloc(nbyte, sizeof(unsigned char));
	char *b64 = (char *)malloc(nbyte * 2);

	for (;;)
	{
		sampleSink->setFrames(xCodec2BitsQueue, bits, xSemaphoreCodec2);

		encode_base64(bits, nbyte, (byte *)b64);
		Serial.print(b64);
		lineLength += strlen(b64);
		if (lineLength > 80)
		{
			Serial.println();
			lineLength = 0;
		}
		taskYIELD();
	}
}

void vCodec2SampleSource(void *pvParameters)
{
	int nbyte = (codec2_bits_per_frame(codec2) + 7) / 8;
	for (;;)
	{
		if (nbit_ctr + nbyte < lookdave_bit_len)
		{
			sampleSource->getFrames(lookdave_bit + nbit_ctr, xAudioSamplesQueue, xSemaphoreCodec2);
			nbit_ctr += nbyte;
		}
		else
		{
			nbit_ctr = 0;
		}
		taskYIELD();
	}
}

void setup()
{
	Serial.begin(115200);
	delay_1s.start(1000, AsyncDelay::MILLIS);
	Serial.printf("Build %s\r\n", __TIMESTAMP__);
	Serial.printf("CPU clock speed: %uMHz\r\n", ESP.getCpuFreqMHz());
	pinMode(iopin, OUTPUT);

	codec2 = codec2_create(CODEC2_MODE_1200);
	codec2_set_natural_or_gray(codec2, 0);

	sampleSource = new Codec2Generator(codec2);
	sampleSink = new Codec2Encoder(codec2);

	xCodec2BitsQueue = xQueueCreate(3, sizeof(uint16_t) * sampleSink->getFrameSampleCount());
	if (xCodec2BitsQueue == NULL)
	{
		Serial.println("Can't create queue");
		while (true)
			;
	}
	xAudioSamplesQueue = xQueueCreate(3, sizeof(Frame_t) * sampleSource->getFrameSampleCount());
	if (xAudioSamplesQueue == NULL)
	{
		Serial.println("Can't create queue");
		while (true)
			;
	}
	xSemaphoreCodec2 = xSemaphoreCreateMutex();
	if (xSemaphoreCodec2 == NULL)
	{
		Serial.println("Can't create semaphore");
		while (true)
			;
	}

	//  codec2_destroy(codec2);

	Serial.println("Starting I2S Output");
	input = new Sgtl5000Sampler(I2S_NUM_0, &i2s_pin_config);
	output = new Sgtl5000_Output(I2S_NUM_0, &i2s_pin_config);
	output->start(sampleSource, xAudioSamplesQueue); //init needed here to generate MCLK, needed for SGTL5000 init.

	Serial.printf("SGTL5000 %s initialized.", audioShield.enable() ? "is" : "not");
	audioShield.volume(0.5);
	audioShield.lineInLevel(2); //2.22Vpp equals maximum output.
	xTaskCreate(vEncoderTask, "Codec2Encoder", 24576, NULL, 2, NULL);
	xTaskCreate(vCodec2SampleSource, "Codec2Decoder", 24576, NULL, 2, NULL);
}

void loop()
{
	if (delay_1s.isExpired())
	{
		delay_1s.repeat(); // Count from when the delay expired, not now
		if (isPlaying)
		{
			output->stop();
			vTaskDelay(1);
			input->start(sampleSink, xCodec2BitsQueue);
		}
		else
		{
			input->stop();
			vTaskDelay(1);
			output->start(sampleSource, xAudioSamplesQueue);
		}
		digitalWrite(iopin, isPlaying ? HIGH : LOW);
		isPlaying = !isPlaying;
	}
}