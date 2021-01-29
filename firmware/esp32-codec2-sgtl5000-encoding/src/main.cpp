/**
 * This function will export 200 codec2 packets to the serial port, which is worth 8s of sound.
 * The concatenated packets will be base16-encoded.
 * Copy these lines of hex data and paste them into a file (e.g. lookdave.hex)
 * You can convert that file to a binary format (e.g. lookdave.bit) using xxd:
 *    	xxd -r -p lookdave.hex lookdave.bit
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

CODEC2 *codec2;
Sgtl5000Sampler *input;
AudioControlSGTL5000 audioShield;
QueueHandle_t xQueue;
unsigned char *bits;
const int CODEC2_PACKET_COUNT = 200; // 200 * 40ms = 8s
int codec2_packetCtr = 0;
int nbyte;

void vEncoderTask(void *pvParameters)
{
	short samples[codec2_samples_per_frame(codec2)];

	for (;;)
	{
		if (xQueueReceive(xQueue, samples, portMAX_DELAY) == pdTRUE)
		{
			if (codec2_packetCtr <= nbyte * (CODEC2_PACKET_COUNT - 1))
			{
				codec2_encode(codec2, bits + codec2_packetCtr, samples);
				codec2_packetCtr += nbyte;
			}
			else
			{
				//all packets received, print them
				int linectr = 0;
				Serial.println();
				for (int i = 0; i < nbyte * CODEC2_PACKET_COUNT; i++)
				{
					Serial.printf("%02x", bits[i]);
					if ((++linectr) % 30 == 0)
					{
						Serial.println();
					}
				}
				Serial.println();
				while (true)
					;
			}
		}
	}
}

void setup()
{
	Serial.begin(115200);

	Serial.printf("Build %s\r\n", __TIMESTAMP__);
	Serial.printf("CPU clock speed: %uMHz\r\n", ESP.getCpuFreqMHz());

	codec2 = codec2_create(CODEC2_MODE_1200);
	codec2_set_natural_or_gray(codec2, 0);
	nbyte = (codec2_bits_per_frame(codec2) + 7) / 8;
	bits = (unsigned char *)calloc(nbyte * CODEC2_PACKET_COUNT, sizeof(unsigned char));

	xQueue = xQueueCreate(3, sizeof(uint16_t) * codec2_samples_per_frame(codec2));
	if (xQueue == NULL)
	{
		Serial.println("Can't create queue");
		while (true)
			;
	}

	//  codec2_destroy(codec2);

	Serial.println("Starting I2S Output");
	input = new Sgtl5000Sampler(I2S_NUM_0, 26, 25, 33);
	input->start(xQueue, codec2_samples_per_frame(codec2));
	Serial.printf("SGTL5000 %s initialized.", audioShield.enable() ? "is" : "not");
	audioShield.lineInLevel(2); //2.22Vpp equals maximum output.
	xTaskCreate(vEncoderTask, "Codec2Encoder", 24576, NULL, 2, NULL);
}

void loop()
{
	// nothing to do here - everything is taken care of by tasks
}