#include <Arduino.h>
#include "lookdave.h"
#include "Codec2Interface.h"
#include "AsyncDelay.h"

const int MODE_SELECT_PIN = 25;
const int TIMER_INTERVAL = 5000;
bool isClient = false;
Codec2Interface c2i;
AsyncDelay pttTimer;

void setup()
{
	Serial.begin(115200);

	unsigned long startTime = millis();
	while (!Serial && (startTime + (10 * 1000) > millis()))
	{
	}
	pttTimer.start(TIMER_INTERVAL, AsyncDelay::MILLIS);
	Serial.printf("Build %s\r\n", __TIMESTAMP__);
	if (!c2i.init())
	{
		Serial.println("Can't init codec2");
		while (true)
			;
	}
	Serial.println("Ready to roll");
}

void encodingSpeed()
{
	int nsam_ctr = 0;
	int nsam = c2i.getAudioSampleCount();
	int nbyte = c2i.getCodec2PacketSize();
	int bufsize = nsam << 1;
	short *buf = (short *)malloc(nsam * sizeof(short));
	unsigned char *bits = (unsigned char *)malloc(nbyte * sizeof(char));
	int packet_ctr = 0;
	unsigned long startTime;
	unsigned long totalTime = 0;
	while (nsam_ctr + bufsize < lookdave_8Khz_raw_len)
	{
		memcpy(buf, lookdave_8Khz_raw + nsam_ctr, bufsize);
		startTime = micros();
		c2i.startEncodingAudio(buf);
		if (c2i.getEncodedAudio(bits))
		{
			totalTime += micros() - startTime;
			packet_ctr++;
			nsam_ctr += bufsize;
		}
	}
	Serial.printf("Average encoding time per packet: %luµs\r\n", totalTime / packet_ctr);
}

void decodingSpeed()
{
	int nbit_ctr = 0;
	int packet_ctr = 0;
	unsigned long startTime;
	unsigned long totalTime = 0;
	int nsam = c2i.getAudioSampleCount();
	int nbyte = c2i.getCodec2PacketSize();
	unsigned char *bits = (unsigned char *)malloc(nbyte * sizeof(char));
	short *buf = (short *)malloc(nsam * sizeof(short));
	while (nbit_ctr + nbyte < lookdave_bit_len)
	{
		memcpy(bits, lookdave_bit + nbit_ctr, nbyte);
		startTime = micros();
		c2i.startDecodingAudio(bits);
		if (c2i.getDecodedAudio(buf))
		{
			totalTime += micros() - startTime;
			packet_ctr++;
			nbit_ctr += nbyte;
		}
	}
	Serial.printf("Average decoding time per packet: %luµs\r\n", totalTime / packet_ctr);
}

void loop()
{
	if (pttTimer.isExpired())
	{
		pttTimer.repeat();
		isClient = !isClient;
	}
	if (isClient)
	{
		encodingSpeed();
	}
	else
	{
		decodingSpeed();
	}
}