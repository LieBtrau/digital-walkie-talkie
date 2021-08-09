#include <Arduino.h>
#include "lookdave.h"
#include "Codec2Interface.h"
#include "RadioInterface.h"
#include "AsyncDelay.h"
#include "SampleSource.h" //to stop compile errors
#include "pinconfig.h"

const int TIMER_INTERVAL = 5000;
const int PACKET_SIZE = 20;
const int PACKET_INTERVAL_ms = 80; //in ms
const int MAX_PACKET = 100;

int packetCount = 0, totalBytes = 0;
float averageRssi = 0, averageSNR = 0;
unsigned long startInterval = 0;
bool isClient = false;
Codec2Interface c2i;
AsyncDelay pttTimer, wperfTimer;
RadioInterface ri(PIN_CS, PIN_IRQ, PIN_SDN);
int nsam;
int nbyte;
unsigned char *bits;
short *buf;
int nbit_ctr = 0;
int audio_packet_ctr = 0;
int nsam_ctr = 0;
unsigned long startTime;
unsigned long totalTime = 0;
uint8_t data[PACKET_SIZE];

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
	nsam = c2i.getAudioSampleCount();
	nbyte = c2i.getCodec2PacketSize();
	bits = (unsigned char *)malloc(nbyte * sizeof(char));
	buf = (short *)malloc(nsam * sizeof(short));
	pinMode(PIN_MODE_SELECT, INPUT_PULLUP);
	isClient = digitalRead(PIN_MODE_SELECT) == HIGH ? true : false;
	Serial.printf("Mode: %s\r\n", isClient ? "Client" : "Server");
	if (isClient)
	{
		wperfTimer.start(PACKET_INTERVAL_ms, AsyncDelay::MILLIS);
	}
	if (!ri.init())
	{
		Serial.println("Can't init radio");
		while (true)
			;
	}
	Serial.println("Ready to roll");
}

void encodingSpeed()
{
	int bufsize = nsam << 1;
	if (nsam_ctr + bufsize < lookdave_8Khz_raw_len)
	{
		memcpy(buf, lookdave_8Khz_raw + nsam_ctr, bufsize);
		startTime = micros();
		c2i.startEncodingAudio(buf);
		if (c2i.getEncodedAudio(bits))
		{
			totalTime += micros() - startTime;
			audio_packet_ctr++;
			nsam_ctr += bufsize;
		}
	}
	else
	{
		if (audio_packet_ctr > 0)
		{
			Serial.printf("Uptime: %lu\tAverage encoding time per packet: %luµs\r\n", millis(), totalTime / audio_packet_ctr);
		}
		nsam_ctr = 0;
		audio_packet_ctr = 0;
		totalTime = 0;
	}
}

void encodingPacket()
{
	int bufsize = nsam << 1;
	if (c2i.isEncoderInputBufferSpaceLeft())
	{
		if (nsam_ctr + bufsize < lookdave_8Khz_raw_len)
		{
			memcpy(buf, lookdave_8Khz_raw + nsam_ctr, bufsize);
			if (c2i.startEncodingAudio(buf))
			{
				nsam_ctr += bufsize;
			}
		}
		else
		{
			nsam_ctr = 0;
		}
	}
}

void decodingSpeed()
{
	if (nbit_ctr + nbyte < lookdave_bit_len)
	{
		memcpy(bits, lookdave_bit + nbit_ctr, nbyte);
		startTime = micros();
		c2i.startDecodingAudio(bits);
		if (c2i.getDecodedAudio(buf))
		{
			totalTime += micros() - startTime;
			audio_packet_ctr++;
			nbit_ctr += nbyte;
		}
	}
	else
	{
		if (audio_packet_ctr > 0)
		{
			Serial.printf("Uptime: %lu\tAverage decoding time per packet: %luµs\r\n", millis(), totalTime / audio_packet_ctr);
		}
		nbit_ctr = 0;
		audio_packet_ctr = 0;
		totalTime = 0;
	}
}

void clientloop()
{
	if (wperfTimer.isExpired())
	{
		wperfTimer.repeat();
		data[0] = packetCount;
		ri.sendPacket(data);
		packetCount = packetCount < MAX_PACKET - 1 ? packetCount + 1 : 0;
	}
}

int state = 0;
void sendPacket()
{
	switch (state)
	{
	case 0:
		if (wperfTimer.isExpired())
		{
			wperfTimer.repeat();
		}
		data[0] = packetCount;
		packetCount = packetCount < MAX_PACKET - 1 ? packetCount + 1 : 0;
		state = 1;
		break;
	case 1:
		if (c2i.cntEncodedFramesAvailable()>0 && c2i.getEncodedAudio(data + 1))
		{
			state = 2;
		}
		break;
	case 2:
		if (c2i.cntEncodedFramesAvailable()>0 && c2i.getEncodedAudio(data + nbyte + 1))
		{
			state = 3;
		}
		break;
	case 3:
		ri.sendPacket(data);
		state = 0;
		break;
	default:
		break;
	}
}

void serverloop()
{
	byte data[100];

	if (ri.receivePacket(data))
	{
		totalBytes += ri.getRxPacketLength();
		Serial.printf("%d-", packetCount);
		packetCount++;
		//averageRssi += rf24.lastRssi();
		if (data[0] == 0)
		{
			int bitrate = (totalBytes << 3) * 1000 / (millis() - startInterval);
			Serial.printf("\r\nTotal bytes : %d\tTotal packets : %d\tBitrate : %d bps", totalBytes, packetCount, bitrate);
			if (packetCount > 0)
			{
				Serial.printf("\tAverage RSSI : %.2f\tAverage SNR : %.2f\r\n", averageRssi / packetCount, averageSNR / packetCount);
			}
			else
			{
				Serial.println();
			}
			packetCount = 0;
			averageRssi = 0;
			averageSNR = 0;
			totalBytes = 0;
			startInterval = millis();
		}
	}
	else
	{
		Serial.println("recv failed");
	}
}

void loop()
{
	// if (pttTimer.isExpired())
	// {
	// 	pttTimer.repeat();
	// 	isClient = !isClient;
	// }
	if (isClient)
	{
		encodingPacket();
		sendPacket();
	}
	else
	{
		//decodingSpeed();
		serverloop();
	}
	vTaskDelay(1);
}