#include "Layer1_SX1278.h"
#include "Layer2.h"
#include "AsyncDelay.h"

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
// DIO0 pin:  	ESP32.39	D2
// RESET pin: 	ESP32.36	D6
// DIO1 pin:  	ESP32.34  	D3				//Clock pin in continuous mode (RadioHead: not used)

#ifdef ARDUINO_NUCLEO_F303K8
SX1278 radio = new Module(A3, D2, D6, D3);
#elif defined(ARDUINO_NodeMCU_32S)
SX1278 radio = new Module(5, 39, 36, 34);
#endif

AsyncDelay wperfTimer;
uint8_t RECEIVER[ADDR_LENGTH] = {0xc0, 0xd3, 0xca, 0xfe};
int counter = 0;
const int DATAGRAM_HEADER = 5;
Layer1Class Layer1(&radio, 0);
Layer2 layer2(&Layer1);

void setup()
{
	Serial.begin(115200);
	delay(200);

	Serial.println("* Initializing LoRa...");

	if (Layer1.init())
	{
		Serial.println(" --> LoRa initialized");
	}
	else
	{
		Serial.println(" --> Failed to initialize LoRa");
		while (true)
			;
	}
	wperfTimer.start(1000, AsyncDelay::MILLIS);
}

void clientloop()
{
	if (wperfTimer.isExpired())
	{
		wperfTimer.repeat();
		// BufferEntry entry;
		// char s[] = "Hallo";
		// memcpy(entry.data, s, sizeof(s));
		// entry.length = sizeof(s);
		// Layer1.txBuffer->write(entry);
		// Layer1.transmit();
		struct Datagram datagram; 
		int msglen = sprintf((char *)datagram.message, "%s,%i", "hello", counter);
		memcpy(datagram.destination, RECEIVER, ADDR_LENGTH);
		datagram.type = 's'; // can be anything, but 's' for 'sensor'
	
		// Send packet
		layer2.writeData(datagram, msglen + DATAGRAM_HEADER);
		counter++;
		Serial.println((char *)datagram.message);
	}
}

void serverloop()
{
	layer2.daemon();
	// if (Layer1.receive() > 0)
	// {
	// 	BufferEntry entry = Layer1.rxBuffer->read();
	// 	Serial.println(entry.data);
	// }
}

void loop()
{
#ifdef ARDUINO_NUCLEO_F303K8
	clientloop();
#elif defined(ARDUINO_NodeMCU_32S)
	serverloop();
#endif
}