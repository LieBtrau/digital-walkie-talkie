#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

// SX1278 has the following connections:
// 3V3        ESP32.3V3
// GND        ESP32.GND
// MOSI pin:  ESP32.23
// MISO pin:  ESP32.19
// SCK pin :  ESP32.18
// NSS pin:   ESP32.5
// RESET pin: ESP32.36	//RadioHead: tie to RST
// DIO0 pin:  ESP32.39
// DIO1 pin:  ESP32.34  //Clock pin in continuous mode (RadioHead: not used)
// DIO2 pin:  ESP32.16  //Data pin in continuous mode (RadioHead: not used)
const int CLIENT_SERVER_PIN = 32;
bool isClientMode;
// rf95_reliable_datagram_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple addressed, reliable messaging server
// with the RHReliableDatagram class, using the RH_RF95 driver to control a RF95 radio.
// It is designed to work with the other example rf95_reliable_datagram_client
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with the RFM95W

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

// Singleton instance of the radio driver
RH_RF95 driver(5, 39);
//RH_RF95 driver(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, SERVER_ADDRESS);

// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB

void setup()
{
	// Rocket Scream Mini Ultra Pro with the RFM95W only:
	// Ensure serial flash is not interfering with radio communication on SPI bus
	//  pinMode(4, OUTPUT);
	//  digitalWrite(4, HIGH);

	Serial.begin(115200);
	while (!Serial)
		; // Wait for serial port to be available
	Serial.printf("Build %s\r\n", __TIMESTAMP__);
	if (!manager.init())
	{
		Serial.println("init failed");
		while (true);
	}
	pinMode(CLIENT_SERVER_PIN, INPUT_PULLUP);
	// Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

	// The default transmitter power is 13dBm, using PA_BOOST.
	// If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
	// you can set transmitter powers from 2 to 20 dBm:
	//  driver.setTxPower(20, false);
	// If you are using Modtronix inAir4 or inAir9, or any other module which uses the
	// transmitter RFO pins and not the PA_BOOST pins
	// then you can configure the power transmitter power for 0 to 15 dBm and with useRFO true.
	// Failure to do that will result in extremely low transmit powers.
	//  driver.setTxPower(14, true);

	// You can optionally require this module to wait until Channel Activity
	// Detection shows no activity on the channel before transmitting by setting
	// the CAD timeout to non-zero:
	//  driver.setCADTimeout(10000);
	Serial.println("All setup");
	isClientMode = digitalRead(CLIENT_SERVER_PIN)==HIGH ? true : false;
	Serial.printf("Mode is %s\n", isClientMode ? "client" : "server");
}

uint8_t serverdata[] = "And hello back to you";
uint8_t clientdata[] = "Hello World!";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void serverloop()
{
	if (manager.available())
	{
		// Wait for a message addressed to us from the client
		uint8_t len = sizeof(buf);
		uint8_t from;
		if (manager.recvfromAck(buf, &len, &from))
		{
			Serial.print("got request from : 0x");
			Serial.print(from, HEX);
			Serial.print(": ");
			Serial.println((char *)buf);

			// Send a reply back to the originator client
			if (!manager.sendtoWait(serverdata, sizeof(serverdata), from))
				Serial.println("sendtoWait failed");
		}
	}
}

void clientloop()
{
	Serial.println("Sending to rf95_reliable_datagram_server");

	// Send a message to manager_server
	if (manager.sendtoWait(clientdata, sizeof(clientdata), SERVER_ADDRESS))
	{
		// Now wait for a reply from the server
		uint8_t len = sizeof(buf);
		uint8_t from;
		if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
		{
			Serial.print("got reply from : 0x");
			Serial.print(from, HEX);
			Serial.print(": ");
			Serial.println((char *)buf);
		}
		else
		{
			Serial.println("No reply, is rf95_reliable_datagram_server running?");
		}
	}
	else
		Serial.println("sendtoWait failed");
	delay(500);
}

void loop()
{
	if(isClientMode)
	{
		clientloop();
	}else
	{
		serverloop();
	}
}
