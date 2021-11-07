/** 
 * Connections
 *  SI4463   	ESP32	Nucleo32
 *  VCC       	3V3		3V3
 * 	GND			GND		GND
 * 	MOSI		23		11
 * 	MISO		19		12
 * 	SCK			18		13
 * 	NSEL		5		A3
 *	IRQ			4		D3
 *	SDN			16		D6
 * 
 * Upload this code to two devices.  The MODE_SELECT_PIN determines which device is client or server.  
 * Leave the MODE_SELECT_PIN open on one device, tie it to ground on the other device.
 * 
 * Results with 4GFSK (Rsymb(sps): 2400=4800bps, Fdev(Hz): 350)
 * ------------------------------------------------------------
 * Packet size	Packet interval	Bitrate
 * [byte]		[ms]			[bps]
 * ------------------------------------
 * 20			80				2000
 * 40			120				2666
 * 100			240				3333
 * 200			400				4000
 * 400			720				4266
 */

#include <Arduino.h>
#include "Si446x.h"
#include "AsyncDelay.h"

AsyncDelay wperfTimer;
unsigned long startInterval = 0;
const int PACKET_SIZE = 400;			 //in bytes
const int PACKET_INTERVAL_ms = 750; //in ms
const int MAX_PACKET_COUNT = 100;
const int MODE_SELECT_PIN = 27;
const int CHANNEL = 0;
int packetCount = 0;
float averageRssi = 0;
int totalBytes = 0;
bool isClient = false;
Si446x si4463;

typedef struct
{
	enum
	{
		PACKET_NONE,
		PACKET_OK,
		PACKET_INVALID
	} ready;
	int16_t rssi;
	word length;
	byte buffer[PACKET_SIZE];
} pingInfo_t;

static volatile pingInfo_t pingInfo;

void onReceiveInvalid(int16_t rssi)
{
	pingInfo.ready = pingInfo_t::PACKET_INVALID;
}

void onReceive(word length)
{
	pingInfo.ready = pingInfo_t::PACKET_OK;
	pingInfo.rssi = si4463.getLatchedRSSI();
	pingInfo.length = length;

	si4463.readBytes((byte *)pingInfo.buffer, length);
	// Radio will now be in idle mode
}

void setup()
{
	Serial.begin(115200);
	delay(1000);
	Serial.printf("Build %s\r\n", __TIMESTAMP__);

	// Start up
	si4463.setPins(5, 4, 16);
	si4463.begin(CHANNEL);
	si4463.setTxPower(50);

	pinMode(LED_BUILTIN, OUTPUT);

	pinMode(MODE_SELECT_PIN, INPUT_PULLUP);
	isClient = digitalRead(MODE_SELECT_PIN) == HIGH ? true : false;
	if (isClient)
	{
		Serial.println("Client");
		wperfTimer.start(PACKET_INTERVAL_ms, AsyncDelay::MILLIS);
	}
	else
	{
		Serial.println("Server");
		si4463.onReceive(onReceive);
		si4463.onReceiveInvalid(onReceiveInvalid);
	}
}

void clientloop()
{
	uint8_t data[PACKET_SIZE]={1};
	if (wperfTimer.isExpired())
	{
		digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) == HIGH ? LOW : HIGH);
		wperfTimer.repeat();
		packetCount = packetCount < MAX_PACKET_COUNT-1 ? packetCount + 1 : 0;
		data[0] = packetCount;
		// Send the data
		si4463.beginPacket();
		si4463.write(data, sizeof(data));
		if (!si4463.endPacket(SI446X_STATE_RX))
		{
			Serial.println("TX error");
		}
	}
}

void serverloop()
{
	// Put into receive mode
	si4463.receive();
	pingInfo.ready = pingInfo_t::PACKET_NONE;
	while (pingInfo.ready == pingInfo_t::PACKET_NONE)
		;

	digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) == HIGH ? LOW : HIGH);
	if (pingInfo.ready == pingInfo_t::PACKET_OK)
	{
		totalBytes += pingInfo.length;
		packetCount++;
		averageRssi += pingInfo.rssi;
		if (pingInfo.buffer[0] == 0)
		{
			int bitrate = (totalBytes << 3) * 1000 / (millis() - startInterval);
			Serial.printf("Total bytes : %d\tTotal packets : %d\tBitrate : %d bps", totalBytes, packetCount, bitrate);
			if (packetCount > 0)
			{
				Serial.printf("\tAverage RSSI : %.2f\r\n", averageRssi / packetCount);
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