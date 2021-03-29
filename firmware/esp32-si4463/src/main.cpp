/*
   RadioLib Si443x Settings Example

   This example shows how to change all the properties of RF69 radio.
   RadioLib currently supports the following settings:
    - pins (SPI slave select, nIRQ, shutdown)
    - carrier frequency
    - bit rate
    - receiver bandwidth
    - frequency deviation
    - output power during transmission
    - sync word

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#si443xrfm2x

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

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

// Singleton instance of the radio driver
#ifdef ARDUINO_NUCLEO_F303K8
RH_RF24 rf24(A3, 3, 6);
#elif defined(ARDUINO_NodeMCU_32S)
RH_RF24 rf24(5, 4, 16);
const int MODE_SELECT_PIN = 25;
#endif

void setup()
{
	Serial.begin(115200);
	delay(1000);
	pinMode(MODE_SELECT_PIN, INPUT_PULLUP);

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
	Serial.println("Sending to rf24_server");
	// Send a message to rf24_server
	uint8_t data[] = "Hello World!";
	if (!rf24.send(data, sizeof(data)) || !rf24.waitPacketSent())
	{
		Serial.println("Packet TX error.");
	}
	// Now wait for a reply
	uint8_t buf[RH_RF24_MAX_MESSAGE_LEN];
	uint8_t len = sizeof(buf);

	if (rf24.waitAvailableTimeout(500))
	{
		// Should be a reply message for us now
		if (rf24.recv(buf, &len))
		{
			Serial.print("got reply: ");
			Serial.println((char *)buf);
		}
		else
		{
			Serial.println("recv failed");
		}
	}
	else
	{
		Serial.println("No reply, is rf24_server running?");
	}
	delay(400);
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
			//      RF24::printBuffer("request: ", buf, len);
			Serial.print("got request: ");
			Serial.println((char *)buf);
			//      Serial.print("RSSI: ");
			//      Serial.println((uint8_t)rf24.lastRssi(), DEC);

			// Send a reply
			uint8_t data[] = "And hello back to you";
			rf24.send(data, sizeof(data));
			rf24.waitPacketSent();
			Serial.println("Sent a reply");
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
	if (digitalRead(MODE_SELECT_PIN))
	{
		serverloop();
	}
	else
	{
		clientloop();
	}
#endif
}