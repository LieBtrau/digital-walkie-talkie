/**
 * CODEC2 ENCODING
 * ---------------
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
 */


#include <Arduino.h>
#include "lookdave.h"
#include "Codec2Interface.h"
#include "Codec2Decoder.h"
#include "Codec2Encoder.h"
#include "Sgtl5000_Output.h"
#include "Sgtl5000_Input.h"
#include "control_sgtl5000.h"
#include "AsyncDelay.h"
#include "pinconfig.h"
#include "base64.hpp"

AsyncDelay delay_3s;
bool isPlaying = true;
Codec2Interface c2i;
Codec2Decoder c2d(&c2i);
Codec2Encoder c2e(&c2i);
int nbit_ctr = 0, nbyte, lineLength = 0;
;
Sgtl5000_Output *output;
Sgtl5000_Input *input;
AudioControlSGTL5000 audioShield;
static i2s_pin_config_t i2s_pin_config = {
	.bck_io_num = PIN_BCLK,	   // Serial Clock (SCK)
	.ws_io_num = PIN_LRCLK,	   // Word Select (WS)
	.data_out_num = PIN_SDOUT, // data out to audio codec
	.data_in_num = PIN_SDIN	   // data from audio codec
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
	if (!c2d.init() || !c2e.init())
	{
		while (true)
		{
		}
	}
	nbyte = c2i.getCodec2PacketSize();
	Serial.println("Starting I2S Output");
	output = new Sgtl5000_Output(I2S_NUM_0, &i2s_pin_config);
	//output->start(&c2d); //init needed here to generate MCLK, needed for SGTL5000 init.
	input = new Sgtl5000_Input(I2S_NUM_0, &i2s_pin_config);
	input->start(&c2e);
	Serial.printf("SGTL5000 %s initialized.\r\n", audioShield.enable() ? "is" : "not");
	audioShield.volume(0.5);
	audioShield.lineInLevel(2); //2.22Vpp equals maximum output.
	Serial.println("Ready to roll");
}

void loop()
{
	// if (delay_3s.isExpired())
	// {
	// 	delay_3s.repeat(); // Count from when the delay expired, not now
	// 	if (isPlaying)
	// 	{
	// 		output->stop();
	// 	}
	// 	else
	// 	{
	// 		output->start(&c2d);
	// 	}
	// 	digitalWrite(LED_BUILTIN, isPlaying ? HIGH : LOW);
	// 	isPlaying = !isPlaying;
	// }
	// if (isPlaying && c2i.isDecodingInputBufferSpaceLeft())
	// {
	// 	//Decoding loop
	// 	byte bits[nbyte];
	// 	memcpy(bits, lookdave_bit + nbit_ctr, nbyte);
	// 	c2i.startDecodingAudio(bits);
	// 	nbit_ctr = nbit_ctr + nbyte < lookdave_bit_len ? nbit_ctr + nbyte : 0;
	// }
	if (isPlaying && c2i.isEncodedFrameAvailable())
	{
		//Encoding loop
		byte bits[nbyte];
		char *b64 = (char *)malloc(nbyte * 2);
		if (c2i.getEncodedAudio(bits))
		{
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
}