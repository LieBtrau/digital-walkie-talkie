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
 */

// rf24_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_RF24 class. RH_RF24 class does not provide for addressing or
// reliability, so you should only use RH_RF24 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf24_server.
// Tested on Anarduino Mini http://www.anarduino.com/mini/ with RFM24W and RFM26W

#include <SPI.h>
#include <RH_RF24.h>
#include "AsyncDelay.h"

// Singleton instance of the radio driver
#ifdef ARDUINO_NUCLEO_F303K8
RH_RF24 rf24(A3, 3, 6);
#elif defined(ARDUINO_NodeMCU_32S)
RH_RF24 rf24(5, 4, 16);
const int MODE_SELECT_PIN = 25;
#endif

AsyncDelay wperfTimer;
unsigned long startInterval=0;
const int PACKET_INTERVAL_ms = 40; //in ms
const int PACKET_SIZE = 6;		   //in bytes
int packetCount = 0;
float averageRssi = 0;
float averageSNR = 0;
int totalBytes = 0;
bool isClient = false;

void setup()
{
	Serial.begin(115200);
	delay(1000);
	Serial.printf("Build %s\r\n", __TIMESTAMP__);
	pinMode(MODE_SELECT_PIN, INPUT_PULLUP);
	isClient = digitalRead(MODE_SELECT_PIN) == HIGH ? true : false;
	if (isClient)
	{
		wperfTimer.start(PACKET_INTERVAL_ms, AsyncDelay::MILLIS);
	}
	Serial.println("* Initializing radio...");
	if (!rf24.init())
		Serial.println("init failed");
	else
		Serial.println("init ok");
	// The default radio config is for 30MHz Xtal, 434MHz base freq 2GFSK 5kbps 10kHz deviation
	// power setting 0x10
	// If you want a different frequency mand or modulation scheme, you must generate a new
	// radio config file as per the RH_RF24 module documentation and recompile
	// You can change a few other things programatically:
	//rf24.setTxPower(0x7f);
}

void clientloop()
{
	uint8_t data[PACKET_SIZE];
	if (wperfTimer.isExpired())
	{
		wperfTimer.repeat();
		data[0] = packetCount++;
		if (packetCount == 250)
		{
			packetCount = 0;
		}
		if (!rf24.send(data, sizeof(data)) || !rf24.waitPacketSent())
		{
			Serial.println("Packet TX error.");
		}
	}
}

void serverloop()
{
	if (rf24.available())
	{
		// Should be a message for us now
		uint8_t buf[RH_RF24_MAX_MESSAGE_LEN];
		uint8_t len = sizeof(buf);
		if (rf24.recv(buf, &len))
		{
			totalBytes += len;
			packetCount++;
			averageRssi += rf24.lastRssi();
			if (buf[0] == 0)
			{
				int bitrate = (totalBytes<<3) * 1000/(millis()-startInterval);
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
}

void loop()
{
#ifdef ARDUINO_NUCLEO_F303K8
	serverloop();
	//clientloop();
#elif defined(ARDUINO_NodeMCU_32S)
	if (!isClient)
	{
		serverloop();
	}
	else
	{
		clientloop();
	}
#endif
}