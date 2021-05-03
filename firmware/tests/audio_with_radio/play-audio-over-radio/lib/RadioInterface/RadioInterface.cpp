#include "RadioInterface.h"

static RadioInterface *ri;

void vRadioTask(void *pvParameters)
{
	for (;;)
	{
		byte packet[RadioInterface::PACKET_SIZE];
		if (xQueueReceive(ri->txPacketsQueue, packet, 10) == pdTRUE)
		{
			if (ri->radio.transmit(packet, sizeof(packet)) != ERR_NONE)
			{
				Serial.println("TX error");
			}
		}
		if (ri->radio.receive(packet, 0) == ERR_NONE)
		{
			xQueueSendToBack(ri->rxPacketsQueue, packet, portMAX_DELAY);
		}
		taskYIELD();
	}
}

bool RadioInterface::sendPacket(byte *data)
{
	xQueueSendToBack(ri->txPacketsQueue, data, portMAX_DELAY);
}

bool RadioInterface::receivePacket(byte *data)
{
	return xQueueReceive(ri->rxPacketsQueue, data, portMAX_DELAY) == pdTRUE;
}

RadioInterface::RadioInterface(/* args */)
{
	ri = this;
}

RadioInterface::~RadioInterface()
{
}

bool RadioInterface::init()
{
	Serial.print(F("[Si4463] Initializing ... "));
	int state = radio.begin();
	if (state == ERR_NONE)
	{
		Serial.println(F("success!"));
	}
	else
	{
		Serial.print(F("failed, code "));
		Serial.println(state);
		while (true)
			;
	}
	//RadioInterface.setTxPower(SI446X_MAX_TX_POWER);
	txPacketsQueue = xQueueCreate(3, PACKET_SIZE);
	if (txPacketsQueue == NULL)
	{
		Serial.println("Can't create queue");
		while (true)
			;
	}
	rxPacketsQueue = xQueueCreate(3, PACKET_SIZE);
	if (rxPacketsQueue == NULL)
	{
		Serial.println("Can't create queue");
		while (true)
			;
	}
	xTaskCreate(vRadioTask, "RadioTask", 2000, NULL, 2, NULL);
	return true;
}