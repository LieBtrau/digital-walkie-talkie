/**
 * It doesn't seem to make any difference at all :
 * - if xqueuesend is called with buf or &buf.
 * - if xqueuereceive is called with bufout or &bufout
 */

#include "Arduino.h"

QueueHandle_t queue;
typedef struct
{
	uint16_t left;
	uint16_t right;
} Frame_t;

const int FRAME_COUNT=100;
Frame_t frames[FRAME_COUNT];

void setup()
{
	Frame_t buf[FRAME_COUNT];

	Serial.begin(115200);

	queue = xQueueCreate(10, sizeof(frames));

	if (queue == NULL)
	{
		Serial.println("Error creating the queue");
	}

	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < FRAME_COUNT; j++)
		{
			buf[j].left = i * j;
			buf[j].right = 0;
		}
		xQueueSend(queue, buf, portMAX_DELAY);
	}

}

void loop()
{

	Frame_t bufout[FRAME_COUNT];

	for (int i = 0; i < 10; i++)
	{
		xQueueReceive(queue, bufout, portMAX_DELAY);
		for (int j = 0; j < 10; j++)
		{
			Serial.print(bufout[j].left);
			Serial.print("|");
		}
		Serial.println();
	}

	Serial.println();
	delay(1000);
}