#include <Arduino.h>
#include "lookdave.h"
#include "Codec2Interface.h"
#include "RadioInterface.h"
#include "AsyncDelay.h"

const int MODE_SELECT_PIN = 27;
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
RadioInterface ri;

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
	pinMode(MODE_SELECT_PIN, INPUT_PULLUP);
	isClient = digitalRead(MODE_SELECT_PIN) == HIGH ? true : false;
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
	Serial.printf("Uptime: %lu\tAverage encoding time per packet: %luµs\r\n", millis(), totalTime / packet_ctr);
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
	Serial.printf("Uptime: %lu\tAverage decoding time per packet: %luµs\r\n", millis(), totalTime / packet_ctr);
}

void clientloop()
{
	uint8_t data[PACKET_SIZE];
	if (wperfTimer.isExpired())
	{
		wperfTimer.repeat();
		data[0] = packetCount++;
		if (packetCount == MAX_PACKET)
		{
			packetCount = 0;
		}
		ri.sendPacket(data);
	}
}

void serverloop()
{
	byte data[100];

	if (ri.receivePacket(data))
	{
		totalBytes += ri.getPacketLength();
		packetCount++;
		//averageRssi += rf24.lastRssi();
		if (data[0] == 0)
		{
			int bitrate = (totalBytes << 3) * 1000 / (millis() - startInterval);
			Serial.printf("Total bytes : %d\tTotal packets : %d\tBitrate : %d bps", totalBytes, packetCount, bitrate);
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
		//encodingSpeed();
		clientloop();
	}
	else
	{
		//decodingSpeed();
		serverloop();
	}
}