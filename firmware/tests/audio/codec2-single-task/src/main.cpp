#include <Arduino.h>
#include "codec2.h"
#include "lookdave.h"

const int mode = CODEC2_MODE_1200;
CODEC2 *codec2;
QueueHandle_t xAudioSamplesQueue = NULL;
QueueHandle_t xCodec2SamplesQueue = NULL;
bool isCodec2Encoding = false;

void codec2task(void *pvParameters)
{
	codec2 = codec2_create(CODEC2_MODE_1200);
	codec2_set_natural_or_gray(codec2, 0);

	int nsam = codec2_samples_per_frame(codec2);
	xAudioSamplesQueue = xQueueCreate(3, sizeof(int16_t) * nsam);
	if (xAudioSamplesQueue == NULL)
	{
		Serial.println("Can't create xAudioSamplesQueue");
		while (true)
			;
	}

	int nbyte = (codec2_bits_per_frame(codec2) + 7) / 8;
	xCodec2SamplesQueue = xQueueCreate(3, nbyte);
	if (xCodec2SamplesQueue == NULL)
	{
		Serial.println("Can't create xCodec2SamplesQueue");
		while (true)
			;
	}

	unsigned char *bits = (unsigned char *)malloc(nbyte * sizeof(char));
	short *buf = (short *)malloc(nsam * sizeof(short));
	for (;;)
	{
		if (isCodec2Encoding)
		{
			//Codec2 encoding : Receive from audio queue and send to codec2 queue
			if (xQueueReceive(xAudioSamplesQueue, buf, 10) == pdTRUE)
			{
				codec2_encode(codec2, bits, buf);
				xQueueSendToBack(xCodec2SamplesQueue, bits, 10);
			}
		}
		else
		{
			//Codec2 decoding : Receive from codec2 bits and send to audio queue
			if (xQueueReceive(xCodec2SamplesQueue, bits, 10) == pdTRUE)
			{
				codec2_decode(codec2, buf, bits);
				xQueueSendToBack(xAudioSamplesQueue, buf, 10);
			}
		}
	}
}

void setup()
{
	Serial.begin(115200);

	unsigned long startTime = millis();
	while (!Serial && (startTime + (10 * 1000) > millis()))
	{
	}
	Serial.printf("Build %s\r\n", __TIMESTAMP__);

	xTaskCreate(codec2task, "Codec2Task", 24576, NULL, 2, NULL);
}

void encodingSpeed()
{
	isCodec2Encoding = true;
	int nsam_ctr = 0;
	int nsam = 320; //todo : variable depending on codec2 mode
	int nbyte = 8;
	int bufsize = nsam << 1;
	short *buf = (short *)malloc(nsam * sizeof(short));
	unsigned char *bits = (unsigned char *)malloc(nbyte * sizeof(char));
	int packet_ctr = 0;
	unsigned long startTime;
	unsigned long totalTime = 0;
	if (xAudioSamplesQueue == NULL || xCodec2SamplesQueue == NULL)
	{
		Serial.println("not ready");
		return;
	}
	while (nsam_ctr + bufsize < lookdave_8Khz_raw_len)
	{
		memcpy(buf, lookdave_8Khz_raw + nsam_ctr, bufsize);
		startTime = micros();
		xQueueSendToBack(xAudioSamplesQueue, buf, 10);
		if (xQueueReceive(xCodec2SamplesQueue, bits, 100) == pdTRUE)
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
	isCodec2Encoding=false;
	int nbit_ctr = 0;
	int packet_ctr = 0;
	unsigned long startTime;
	unsigned long totalTime = 0;
	int nsam = 320; //todo : variable depending on codec2 mode
	int nbyte = 8;
	unsigned char *bits = (unsigned char *)malloc(nbyte * sizeof(char));
	short *buf = (short *)malloc(nsam * sizeof(short));
	if (xAudioSamplesQueue == NULL || xCodec2SamplesQueue == NULL)
	{
		Serial.println("not ready");
		return;
	}
	while (nbit_ctr + nbyte < lookdave_bit_len)
	{
		memcpy(bits, lookdave_bit + nbit_ctr, nbyte);
		startTime = micros();
		xQueueSendToBack(xCodec2SamplesQueue, bits, 10);
		if (xQueueReceive(xAudioSamplesQueue, buf, 100) == pdTRUE)
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
	// put your main code here, to run repeatedly:
	//encodingSpeed();
	decodingSpeed();
	delay(500);
}