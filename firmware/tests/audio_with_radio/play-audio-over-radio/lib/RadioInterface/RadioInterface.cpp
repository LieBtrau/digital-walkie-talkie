#include "RadioInterface.h"

static RadioInterface *ri;

void vRadioTask(void *pvParameters)
{
	for (;;)
	{
		byte packet[RadioInterface::PACKET_SIZE];
		if (xQueueReceive(ri->txPacketsQueue, packet, 1) == pdTRUE)
		{
			if (ri->radio.transmit(packet, sizeof(packet)) != ERR_NONE)
			{
				Serial.println("TX error");
			}
		}
		if (ri->receiveActive && ri->radio.pollIRQLine() && ri->radio.readData(packet, 10) == ERR_NONE)
		{
			xQueueSendToBack(ri->rxPacketsQueue, packet, portMAX_DELAY);
		}
	}
}

bool RadioInterface::sendPacket(byte *data)
{
	return xQueueSendToBack(ri->txPacketsQueue, data, portMAX_DELAY) == pdTRUE;
}

bool RadioInterface::receivePacket(byte *data)
{
	ri->radio.startReceive();
	receiveActive = true;
	bool success = xQueueReceive(ri->rxPacketsQueue, data, (TickType_t)500) == pdTRUE;
	receiveActive = false;
	return success;
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

int RadioInterface::getPacketLength()
{
	return radio.getPacketLength();
}