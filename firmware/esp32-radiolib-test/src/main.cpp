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
// DIO0 pin:  	ESP32.39	D7				//Rising edge triggers interrupt, D2 doesn't work for Nucleo32!
// RESET pin: 	ESP32.36	D6
// DIO1 pin:  	ESP32.34  	D3				//Clock pin in continuous mode (RadioHead: not used)

#ifdef ARDUINO_NUCLEO_F303K8
bool useTxInterrtupt = true;
bool useRxInterrupt = false;
SX1278 radio = new Module(A3, D7, D6, D3);
#elif defined(ARDUINO_NodeMCU_32S)
bool useTxInterrtupt = false;
bool useRxInterrupt = true;
SX1278 radio = new Module(5, 39, 36, 34);
#endif

AsyncDelay wperfTimer;
// save transmission state between loops
int transmissionState = ERR_NONE;
// flag to indicate that a packet was sent
volatile bool transmittedFlag = false;

// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;
volatile uint16_t irqFlags;

// this function is called when a complete packet
// is transmitted by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void)
{
	irqFlags = radio.getIRQFlags();
	//bit 14 : Fifo empty
	//bit 11 : PacketSent
	//bit 10 : RX payload ready
	//bit 9 : CRC ok
	//bit 7 : mode ready
	//bit 6 : rx ready
	//bit 5 : TX ready
	//bit 4 : PLL locked
	//bit 3 : RSSI threshold exceeded
	//bit 1 : preamble detected
	//bit 0 : syncaddress match

	Serial.println(irqFlags, HEX);
	// check if the interrupt is enabled
	if (!enableInterrupt)
	{
		return;
	}

	if (useTxInterrtupt)
	{
		// we sent a packet, set the flag
		transmittedFlag = true;
		radio.standby(); //power off to save power
	}
	if (useRxInterrupt)
	{
		// we got a packet, set the flag
		receivedFlag = true;
	}
}

void setup()
{
	Serial.begin(115200);
	// initialize SX1278 with default settings
	Serial.print(F("[SX1278] Initializing ... "));
	int state = radio.beginFSK();
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

	if (useTxInterrtupt)
	{
		// set the function that will be called
		// when packet transmission is finished
		radio.setDio0Action(setFlag);

		// start transmitting the first packet
		Serial.print(F("[SX1278] Sending first packet ... "));

		// you can transmit C-string or Arduino string up to
		// 256 characters long
		transmissionState = radio.startTransmit("Hello World!");
	}
	if (useRxInterrupt)
	{
		// set the function that will be called
		// when new packet is received
		radio.setDio0Action(setFlag);

		// start listening for LoRa packets
		Serial.print(F("[SX1278] Starting to listen ... "));
		state = radio.startReceive();
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
}

void serverloop()
{
	Serial.print(F("[SX1278] Waiting for incoming transmission ... "));

	// you can receive data as an Arduino String
	// NOTE: receive() is a blocking method!
	//       See example ReceiveInterrupt for details
	//       on non-blocking reception method.
	// String str;
	// int state = radio.receive(str);

	// you can also receive data as byte array
	byte byteArr[8];
	int state = radio.receive(byteArr, 8);

	if (state == ERR_NONE)
	{
		// packet was successfully received
		Serial.println(F("success!"));

		// print the data of the packet
		Serial.print(F("[SX1278] Data:\t\t\t"));
		//Serial.println(str);

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

void serverInterruptloop()
{
	// check if the flag is set
	if (receivedFlag)
	{
		// disable the interrupt service routine while
		// processing the data
		enableInterrupt = false;

		// reset flag
		receivedFlag = false;

		// you can read received data as an Arduino String
		// String str;
		// int state = radio.readData(str);

		// you can also read received data as byte array
		byte byteArr[20];
		int state = radio.readData(byteArr, 20); //works also if expected length is longer than expected length

		if (state == ERR_NONE)
		{
			// packet was successfully received
			Serial.println(F("[SX1278] Received packet!"));

			// print data of the packet
			Serial.print(F("[SX1278] Data:\t\t"));
			//Serial.println(str);

			// print RSSI (Received Signal Strength Indicator)
			Serial.print(F("[SX1278] RSSI:\t\t"));
			Serial.print(radio.getRSSI());
			Serial.println(F(" dBm"));

			// print SNR (Signal-to-Noise Ratio)
			Serial.print(F("[SX1278] SNR:\t\t"));
			Serial.print(radio.getSNR());
			Serial.println(F(" dB"));

			// print frequency error
			Serial.print(F("[SX1278] Frequency error:\t"));
			Serial.print(radio.getFrequencyError());
			Serial.println(F(" Hz"));
		}
		else if (state == ERR_CRC_MISMATCH)
		{
			// packet was received, but is malformed
			Serial.println(F("[SX1278] CRC error!"));
		}
		else
		{
			// some other error occurred
			Serial.print(F("[SX1278] Failed, code "));
			Serial.println(state);
		}

		// put module back to listen mode
		radio.startReceive();

		// we're ready to receive more packets,
		// enable interrupt service routine
		enableInterrupt = true;
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

void clientInterruptloop()
{
	// check if the previous transmission finished
	if (transmittedFlag)
	{
		// disable the interrupt service routine while
		// processing the data
		enableInterrupt = false;

		// reset flag
		transmittedFlag = false;

		if (transmissionState == ERR_NONE)
		{
			// packet was successfully sent
			Serial.println(F("transmission finished!"));

			// NOTE: when using interrupt-driven transmit method,
			//       it is not possible to automatically measure
			//       transmission data rate using getDataRate()
		}
		else
		{
			Serial.print(F("failed, code "));
			Serial.println(transmissionState);
		}

		// wait a second before transmitting again
		delay(1000);

		// send another one
		Serial.print(F("[SX1278] Sending another packet ... "));

		// you can transmit C-string or Arduino string up to
		// 256 characters long
		transmissionState = radio.startTransmit("Hello World!");

		// you can also transmit byte array up to 256 bytes long
		/*
      byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
                        0x89, 0xAB, 0xCD, 0xEF};
      int state = radio.startTransmit(byteArr, 8);
    */

		// we're ready to send more packets,
		// enable interrupt service routine
		enableInterrupt = true;
	}
}

void loop()
{
#ifdef ARDUINO_NUCLEO_F303K8
	if (useTxInterrtupt)
	{
		clientInterruptloop();
	}
	else
	{
		clientloop();
	}
#elif defined(ARDUINO_NodeMCU_32S)
	if (useRxInterrupt)
	{
		serverInterruptloop();
	}
	else
	{
		serverloop();
	}
#endif
}