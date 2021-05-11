#include <Arduino.h>
#include "lookdave.h"
#include "Codec2Interface.h"
#include "Codec2Decoder.h"
#include "Sgtl5000_Output.h"
#include "control_sgtl5000.h"
#include "AsyncDelay.h"

AsyncDelay delay_3s;
bool isPlaying = true;
Codec2Interface c2i;
Codec2Decoder c2d(&c2i);
int nbit_ctr = 0, nbyte;
Sgtl5000_Output *output;
AudioControlSGTL5000 audioShield;
static i2s_pin_config_t i2s_pin_config = {
	.bck_io_num = 26,	// Serial Clock (SCK)
	.ws_io_num = 25,	// Word Select (WS)
	.data_out_num = 32, // data out to audio codec
	.data_in_num = 33	// data from audio codec
};

void setup()
{
	Serial.begin(115200);
	delay_3s.start(3000, AsyncDelay::MILLIS);
	pinMode(LED_BUILTIN, OUTPUT);
	unsigned long startTime = millis();
	while (!Serial && (startTime + (10 * 1000) > millis()))
	{
	}
	Serial.printf("Build %s\r\n", __TIMESTAMP__);
	if (!c2d.init())
	{
		while (true)
		{
		}
	}
	nbyte = c2i.getCodec2PacketSize();
	Serial.println("Starting I2S Output");
	output = new Sgtl5000_Output(I2S_NUM_0, &i2s_pin_config);
	output->start(&c2d); //init needed here to generate MCLK, needed for SGTL5000 init.
	Serial.printf("SGTL5000 %s initialized.\r\n", audioShield.enable() ? "is" : "not");
	audioShield.volume(0.5);
	Serial.println("Ready to roll");
}

void loop()
{
	if (delay_3s.isExpired())
	{
		delay_3s.repeat(); // Count from when the delay expired, not now
		if (isPlaying)
		{
			output->stop();
		}
		else
		{
			output->start(&c2d);
		}
		digitalWrite(LED_BUILTIN, isPlaying ? HIGH : LOW);
		isPlaying = !isPlaying;
	}
	if (isPlaying && c2i.isDecodingInputBufferSpaceLeft())
	{
		byte bits[nbyte];
		memcpy(bits, lookdave_bit + nbit_ctr, nbyte);
		c2i.startDecodingAudio(bits);
		nbit_ctr = nbit_ctr + nbyte < lookdave_bit_len ? nbit_ctr + nbyte : 0;
	}
}