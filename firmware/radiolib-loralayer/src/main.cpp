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
const int CLIENT_SERVER_PIN = D7;
#elif defined(ARDUINO_NodeMCU_32S)
SX1278 radio = new Module(5, 39, 36, 34);
const int CLIENT_SERVER_PIN = 32;
#endif

AsyncDelay wperfTimer;
uint8_t RECEIVER[ADDR_LENGTH] = {0xc0, 0xd3, 0xca, 0xfe};
int counter = 0;
const int DATAGRAM_HEADER = 5;
bool isClientMode;
Layer1Class *Layer1;
Layer2 *layer2;

void setup()
{
	Serial.begin(115200);
	delay(200);

	Serial.println("* Initializing LoRa...");

	Layer1 = new Layer1Class(&radio, 0);
	if (Layer1->init())
	{
		Serial.println(" --> LoRa initialized");
		//layer2 = new Layer2(Layer1);
	}
	else
	{
		Serial.println(" --> Failed to initialize LoRa");
		while (true)
			;
	}
	wperfTimer.start(1000, AsyncDelay::MILLIS);
	pinMode(CLIENT_SERVER_PIN, INPUT_PULLUP);
	isClientMode = digitalRead(CLIENT_SERVER_PIN) == HIGH ? true : false;
	Serial.printf("Mode is %s\n", isClientMode ? "client" : "server");
}

void clientloop()
{
	if (wperfTimer.isExpired())
	{
		wperfTimer.repeat();
		BufferEntry entry;
		char s[] = "Hallo";
		memcpy(entry.data, s, sizeof(s));
		entry.length = sizeof(s);
		Layer1->txBuffer->write(entry);
		Layer1->transmit();
		// 	msglen = sprintf((char *)datagram.message, "%s,%i", "hello", counter);
		// 	memcpy(datagram.destination, RECEIVER, ADDR_LENGTH);
		// 	datagram.type = 's'; // can be anything, but 's' for 'sensor'
		// 	datagramsize = msglen + DATAGRAM_HEADER;

		// 	// Send packet
		// 	layer2->writeData(datagram, datagramsize);
		// 	counter++;
		// 	Serial.println((char *)datagram.message);
	}
}

void serverloop()
{
	if (Layer1->receive() > 0)
	{
		BufferEntry entry = Layer1->rxBuffer->read();
		Serial.println(entry.data);
	}
}

void loop()
{
	if (isClientMode)
	{
		clientloop();
	}
	else
	{
		serverloop();
	}
}