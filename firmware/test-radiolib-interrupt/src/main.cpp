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
// DIO0 pin:  	ESP32.39	D7
// RESET pin: 	ESP32.36	D6
// DIO1 pin:  	ESP32.34  	D3				//Clock pin in continuous mode (RadioHead: not used)

#ifdef ARDUINO_NUCLEO_F303K8
SX1278 radio = new Module(A3, D7, D6);
#elif defined(ARDUINO_NodeMCU_32S)
SX1278 radio = new Module(5, 39, 36);
const int CLIENT_SERVER_PIN = 32;
#else
#error Unsupported platform
#endif
float frequency=434.0F;
#ifdef ARDUINO_NodeMCU_32S
// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;



// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void)
{
	Serial.println("..");
	// check if the interrupt is enabled
	if (!enableInterrupt)
	{
		return;
	}

	// we got a packet, set the flag
	receivedFlag = true;
}
void setup()
{
	Serial.begin(115200);
	// initialize SX1278 with default settings
	Serial.print(F("[SX1278] Initializing ... "));
	int state = radio.begin(frequency);
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

	// if needed, 'listen' mode can be disabled by calling
	// any of the following methods:
	//
	// radio.standby()
	// radio.sleep()
	// radio.transmit();
	// radio.receive();
	// radio.readData();
	// radio.scanChannel();
}

void loop()
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
		String str;
		int state = radio.readData(str);

		// you can also read received data as byte array
		/*
      byte byteArr[8];
      int state = radio.readData(byteArr, 8);
    */

		if (state == ERR_NONE)
		{
			// packet was successfully received
			Serial.println(F("[SX1278] Received packet!"));

			// print data of the packet
			Serial.print(F("[SX1278] Data:\t\t"));
			Serial.println(str);

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
			float ferror=radio.getFrequencyError();
			Serial.print(ferror);
			Serial.println(F(" Hz"));

			frequency-=ferror/2e6;
			radio.setFrequency(frequency);
			Serial.printf("%fMHz\r\n",frequency);
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
		if (radio.startReceive() != ERR_NONE)
		{
			Serial.println("startreceive failed");
		}

		// we're ready to receive more packets,
		// enable interrupt service routine
		enableInterrupt = true;
	}
}
#else
// flag to indicate that a packet was sent
volatile bool transmittedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;

// this function is called when a complete packet
// is transmitted by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void)
{
	// check if the interrupt is enabled
	if (!enableInterrupt)
	{
		return;
	}

	// we sent a packet, set the flag
	transmittedFlag = true;
}
// save transmission state between loops
int transmissionState = ERR_NONE;

void setup()
{
	Serial.begin(115200);

	// initialize SX1278 with default settings
	Serial.print(F("[SX1278] Initializing ... "));
	int state = radio.begin(frequency);
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

	// set the function that will be called
	// when packet transmission is finished
	radio.setDio0Action(setFlag);
	/*	Using the frequency error indication (FEI), we can adjust the frequency of this module to get it close to the 
	 *	frequency of the other module.
	 *	Multiple modules need to be synced, then we better sync them to a reference clock.
	 */

	// start transmitting the first packet
	Serial.print(F("[SX1278] Sending first packet ... "));

	// you can transmit C-string or Arduino string up to
	// 256 characters long
	transmissionState = radio.startTransmit("Hello World!");

	// you can also transmit byte array up to 256 bytes long
	/*
    byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
                      0x89, 0xAB, 0xCD, 0xEF};
    state = radio.startTransmit(byteArr, 8);
  */
}

void loop()
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

#endif