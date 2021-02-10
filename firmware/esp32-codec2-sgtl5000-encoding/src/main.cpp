/**
 * This function will export codec2 packets to the serial port,
 * The concatenated packets are be base64-encoded.
 * Copy these lines of data and paste them into a file (e.g. lookdave.b64)
 * You can convert that file to a binary format (e.g. lookdave.bit) using base64:
 *    	base64 -d lookdave.b64 > lookdave.bit
 * This bit-file can then be decoded back to raw audio using :
 * 		c2dec 1200 lookdave.bit lookdave.raw
 * The raw file can then be converted to audio using: 
 * 		sox -e signed-integer -b 16 -r 8000 -c 1 lookdave.raw lookdave.wav
 * 
 * Minicom is too slow for printing in the hex data in real time
 * 	1200bps = 150bytes/s -> 300bytes/s hex print 
 *
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

#include "codec2.h"
#include "Sgtl5000Sampler.h"
#include "control_sgtl5000.h"
#include "base64.hpp"
#include "AsyncDelay.h"
#include "Codec2Encoder.h"

CODEC2 *codec2;
Sgtl5000Sampler *input;
AudioControlSGTL5000 audioShield;
QueueHandle_t xQueue;
int lineLength = 0;
SemaphoreHandle_t xSemaphoreCodec2 = NULL;
AsyncDelay delay_3s;
bool isPlaying = true;
const int iopin = 4; //maximum 3.3MHz digitalWrite toggle frequency.
SampleSink *sampleSink;

static i2s_pin_config_t i2s_pin_config =
	{
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
		sampleSink->setFrames(xQueue, bits, xSemaphoreCodec2);

		encode_base64(bits, nbyte, (byte *)b64);
		Serial.print(b64);
		lineLength += strlen(b64);
		if (lineLength > 80)
		{
			Serial.println();
			lineLength = 0;
		}
	}
}

void setup()
{
	Serial.begin(115200);
	delay_3s.start(3000, AsyncDelay::MILLIS);
	pinMode(iopin, OUTPUT);
	Serial.printf("Build %s\r\n", __TIMESTAMP__);
	Serial.printf("CPU clock speed: %uMHz\r\n", ESP.getCpuFreqMHz());

	codec2 = codec2_create(CODEC2_MODE_1200);
	codec2_set_natural_or_gray(codec2, 0);
	sampleSink = new Codec2Encoder(codec2);

	xQueue = xQueueCreate(3, sizeof(uint16_t) * codec2_samples_per_frame(codec2));
	if (xQueue == NULL)
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

	Serial.println("Starting I2S Input");
	// The pin config as per the setup

	input = new Sgtl5000Sampler(I2S_NUM_0, &i2s_pin_config);
	input->start(sampleSink, xQueue);
	Serial.printf("SGTL5000 %s initialized.\n", audioShield.enable() ? "is" : "not");
	audioShield.lineInLevel(2); //2.22Vpp equals maximum output.
	xTaskCreate(vEncoderTask, "Codec2Encoder", 24576, NULL, 2, NULL);
}

void loop()
{
	if (delay_3s.isExpired())
	{
		delay_3s.repeat(); // Count from when the delay expired, not now
		if (isPlaying)
		{
			input->stop();
		}
		else
		{
			input->start(sampleSink, xQueue);
		}
		digitalWrite(iopin, isPlaying ? HIGH : LOW);
		isPlaying = !isPlaying;
	}
}