#include <Arduino.h>
#include <RadioLib.h>
#include "AsyncDelay.h"
#include "pinconfig.h"
#include "Wire.h" //For I2C LCD
#include "LiquidCrystal_I2C.h"
#include "BluetoothSerial.h"

Si446x radio = new Module(PIN_CS, PIN_IRQ, PIN_SDN);
AsyncDelay packetIntervalTimer;
AsyncDelay measurementIntervalTimer;
unsigned long startInterval = 0;
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

const int PACKET_INTERVAL_ms = 160;		  //in ms
const int MEASUREMENT_INTERVAL_ms = 8000; //in ms
const int PACKET_SIZE = 60;				  //in bytes
const int MAX_PACKET = 100;
int packetCount = 0;
float averageRssi = 0;
int totalBytes = 0;
bool isClient = false;
QueueHandle_t txPacketsQueue;
QueueHandle_t rxPacketsQueue;
volatile bool vRadioTaskReady = false;
BluetoothSerial SerialBT;

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
	//radio.setTxPower(22); //about 10dbm
	radio.setTxPower(0); //about -32dbm
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
	vRadioTaskReady = true;
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
		packetIntervalTimer.start(PACKET_INTERVAL_ms, AsyncDelay::MILLIS);
		Serial.println("client mode");
		SerialBT.begin("client");
	}
	else
	{
		measurementIntervalTimer.start(MEASUREMENT_INTERVAL_ms, AsyncDelay::MILLIS);
		Serial.println("server mode");
		SerialBT.begin("server");
	}
	xTaskCreate(vRadioTask, "RadioTask", 2000, NULL, 2, NULL);
	while (!vRadioTaskReady)
		;
	Serial.println("Ready for looping");
}

void clientloop()
{
	uint8_t data[PACKET_SIZE];
	if (packetIntervalTimer.isExpired())
	{
		packetIntervalTimer.repeat();
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

	if (xQueueReceive(rxPacketsQueue, data, 2000) == pdTRUE)
	{
		totalBytes += radio.getPacketLength();
		packetCount++;
		averageRssi += radio.getLatchedRssi();
		float packetLoss = 1.0f - packetCount * PACKET_INTERVAL_ms / (float)MEASUREMENT_INTERVAL_ms;
		if (measurementIntervalTimer.isExpired())
		{
			measurementIntervalTimer.restart();
			int bitrate = (totalBytes << 3) * 1000 / MEASUREMENT_INTERVAL_ms;
			Serial.printf("Total bytes : %d\tPacket loss : %.0f%%\tBitrate : %d bps", totalBytes, packetLoss * 100.0f, bitrate);
			SerialBT.printf("Total bytes : %d\tPacket loss : %.0f%%\tBitrate : %d bps", totalBytes, packetLoss * 100.0f, bitrate);
			if (packetCount > 0)
			{
				Serial.printf("\tAverage RSSI : %.2fdBm\r\n", averageRssi / packetCount);
				SerialBT.printf("\tAverage RSSI : %.2fdBm\r\n", averageRssi / packetCount);
			}
			else
			{
				Serial.println();
			}
			packetCount = 0;
			averageRssi = 0;
			totalBytes = 0;
			startInterval = millis();
		}
	}
	else
	{
		Serial.println("recv failed");
		SerialBT.println("recv failed");
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