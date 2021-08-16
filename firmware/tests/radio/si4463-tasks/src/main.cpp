#include <Arduino.h>
#include <RadioLib.h>
#include "AsyncDelay.h"
#include "pinconfig.h"

Si446x radio = new Module(PIN_CS, PIN_IRQ, PIN_SDN);
AsyncDelay wperfTimer;
unsigned long startInterval = 0;
const int PACKET_INTERVAL_ms = 80; //in ms
const int PACKET_SIZE = 20;		   //in bytes
const int MAX_PACKET = 100;
int packetCount = 0;
float averageRssi = 0;
float averageSNR = 0;
int totalBytes = 0;
bool isClient = false;
QueueHandle_t txPacketsQueue;
QueueHandle_t rxPacketsQueue;
volatile bool vRadioTaskReady=false;

void vRadioTask(void *pvParameters)
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
	//radio.setTxPower(SI446X_MAX_TX_POWER);
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
	vRadioTaskReady=true;
	for (;;)
	{
		byte packet[PACKET_SIZE];
		if (xQueueReceive(txPacketsQueue, packet, 10) == pdTRUE)
		{
			if (radio.transmit(packet, sizeof(packet)) != ERR_NONE)
			{
				Serial.println("TX error");
			}
		}
		if (!isClient)
		{
			uint8_t success = radio.receive(packet, 0);
			if (success == ERR_NONE)
			{
				xQueueSendToBack(rxPacketsQueue, packet, portMAX_DELAY);
			}
		}
		else
		{
			//Client shouldn't waste time waiting for packets that won't come.
		}
		taskYIELD();
	}
}

void setup()
{
	Serial.begin(115200);
	delay(1000);
	Serial.printf("Build %s\r\n", __TIMESTAMP__);
	pinMode(PIN_MODE_SELECT, INPUT_PULLUP);
	isClient = digitalRead(PIN_MODE_SELECT) == HIGH ? true : false;
	if (isClient)
	{
		wperfTimer.start(PACKET_INTERVAL_ms, AsyncDelay::MILLIS);
		Serial.println("client mode");
	}
	else
	{
		Serial.println("server mode");
	}
	xTaskCreate(vRadioTask, "RadioTask", 2000, NULL, 2, NULL);
	while(!vRadioTaskReady);
	Serial.println("Ready for looping");
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
		xQueueSendToBack(txPacketsQueue, data, portMAX_DELAY);
	}
}

void serverloop()
{
	byte data[100];

	if (xQueueReceive(rxPacketsQueue, data, portMAX_DELAY) == pdTRUE)
	{
		totalBytes += radio.getPacketLength();
		packetCount++;
		averageRssi += radio.getLatchedRssi();
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
	if (!isClient)
	{
		serverloop();
	}
	else
	{
		clientloop();
	}
}