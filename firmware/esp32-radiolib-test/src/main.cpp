#include <Arduino.h>
#include "AsyncDelay.h"
#include "RadioLib.h"

// SX1278 has the following connections:
//--------------------------------------
// SX1278		ESP32		Nucleo32
// ------		-----		--------
// 3V3        	ESP32.3V3	3V3
// GND        	ESP32.GND	GND
// MOSI pin:  	ESP32.23	D11
// MISO pin:  	ESP32.19	D12
// SCK pin :  	ESP32.18	D13
// NSS pin:   	ESP32.5		A3
// DIO0 pin:  	ESP32.39	D2				//Rising edge triggers interrupt
// RESET pin: 	ESP32.36	D6
// DIO1 pin:  	ESP32.34  D3				//Clock pin in continuous mode (RadioHead: not used)

#ifdef ARDUINO_NUCLEO_F303K8
SX1278 radio = new Module(A3, D2, D6, D3);
#elif defined(ARDUINO_NodeMCU_32S)
SX1278 radio = new Module(5, 39, 36, 34);
#endif

AsyncDelay wperfTimer;

void setup()
{
	Serial.begin(115200);
	// initialize SX1278 with default settings
	Serial.print(F("[SX1278] Initializing ... "));
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
}

void serverloop()
{
	Serial.print(F("[SX1278] Waiting for incoming transmission ... "));

	// you can receive data as an Arduino String
	// NOTE: receive() is a blocking method!
	//       See example ReceiveInterrupt for details
	//       on non-blocking reception method.
	String str;
	int state = radio.receive(str);

	// you can also receive data as byte array
	/*
    byte byteArr[8];
    int state = radio.receive(byteArr, 8);
  */

	if (state == ERR_NONE)
	{
		// packet was successfully received
		Serial.println(F("success!"));

		// print the data of the packet
		Serial.print(F("[SX1278] Data:\t\t\t"));
		Serial.println(str);

		// print the RSSI (Received Signal Strength Indicator)
		// of the last received packet
		Serial.print(F("[SX1278] RSSI:\t\t\t"));
		Serial.print(radio.getRSSI());
		Serial.println(F(" dBm"));

		// print the SNR (Signal-to-Noise Ratio)
		// of the last received packet
		Serial.print(F("[SX1278] SNR:\t\t\t"));
		Serial.print(radio.getSNR());
		Serial.println(F(" dB"));

		// print frequency error
		// of the last received packet
		Serial.print(F("[SX1278] Frequency error:\t"));
		Serial.print(radio.getFrequencyError());
		Serial.println(F(" Hz"));
	}
	else if (state == ERR_RX_TIMEOUT)
	{
		// timeout occurred while waiting for a packet
		Serial.println(F("timeout!"));
	}
	else if (state == ERR_CRC_MISMATCH)
	{
		// packet was received, but is malformed
		Serial.println(F("CRC error!"));
	}
	else
	{
		// some other error occurred
		Serial.print(F("failed, code "));
		Serial.println(state);
	}
}

void clientloop()
{
	Serial.print(F("[SX1278] Transmitting packet ... "));

	// you can transmit C-string or Arduino string up to
	// 256 characters long
	// NOTE: transmit() is a blocking method!
	//       See example SX127x_Transmit_Interrupt for details
	//       on non-blocking transmission method.
	int state = radio.transmit("Hello World!");

	// you can also transmit byte array up to 256 bytes long
	/*
    byte byteArr[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    int state = radio.transmit(byteArr, 8);
  */

	if (state == ERR_NONE)
	{
		// the packet was successfully transmitted
		Serial.println(F(" success!"));

		// print measured data rate
		Serial.print(F("[SX1278] Datarate:\t"));
		Serial.print(radio.getDataRate());
		Serial.println(F(" bps"));
	}
	else if (state == ERR_PACKET_TOO_LONG)
	{
		// the supplied packet was longer than 256 bytes
		Serial.println(F("too long!"));
	}
	else if (state == ERR_TX_TIMEOUT)
	{
		// timeout occurred while transmitting packet
		Serial.println(F("timeout!"));
	}
	else
	{
		// some other error occurred
		Serial.print(F("failed, code "));
		Serial.println(state);
	}

	// wait for a second before transmitting again
	delay(1000);
}


void loop()
{
#ifdef ARDUINO_NUCLEO_F303K8
	clientloop();
#elif defined(ARDUINO_NodeMCU_32S)
	serverloop();
#endif
}