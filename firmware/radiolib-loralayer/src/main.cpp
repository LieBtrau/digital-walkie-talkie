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
// DIO0 pin:  	ESP32.39	D7
// RESET pin: 	ESP32.36	D6
// DIO1 pin:  	ESP32.34  	D3				//Clock pin in continuous mode (RadioHead: not used)

#ifdef ARDUINO_NUCLEO_F303K8
SX1278 radio = new Module(A3, D7, D6, D3);
#elif defined(ARDUINO_NodeMCU_32S)
SX1278 radio = new Module(5, 39, 36, 34);
#endif

AsyncDelay wperfTimer;
uint8_t RECEIVER[ADDR_LENGTH] = {0xc0, 0xd3, 0xca, 0xfe};
int counter = 0;
const int DATAGRAM_HEADER = 5;
Layer1 *layer1;
//Layer2 layer2(&Layer1);

const int MEASURE_INTERVAL_ms = 10000;
int packetCount = 0;
float averageRssi = 0;
float averageSNR = 0;
int totalBytes = 0;

void setup()
{
	Serial.begin(115200);
	delay(1000);

	Serial.println("* Initializing radio...");
	layer1 = new Layer1_SX1278(&radio, 1, 7);
	int state = layer1->init();
	if (state == ERR_NONE)
	{
		Serial.println(" --> radio initialized");
	}
	else
	{
		Serial.printf(" --> Failed to initialize radio: %d\r\n", state);
		while (true)
			;
	}
	// if(radio.fixedPacketLengthMode(40)!=ERR_NONE || radio.setCRC(true)!=ERR_NONE)
	// {
	// 	Serial.println("Can't set fix packet mode.");
	// 	while (true)
	// 	{
	// 		/* code */
	// 	}

	// }
	wperfTimer.start(MEASURE_INTERVAL_ms, AsyncDelay::MILLIS);
	//Layer1 actions
}

void clientloop()
{
	BufferEntry entry;
	byte s[20] = {0};
	memcpy(entry.data, s, sizeof(s));
	entry.length = sizeof(s);

	layer1->txBuffer->write(entry);
	int state = layer1->transmit();
	if (state < 0)
	{
		Serial.printf("TX error: %d\r\n", state);
	}
	//layer1->receive();
	delay(1000);
	//Layer2 actions
	// struct Datagram datagram;
	// int msglen = sprintf((char *)datagram.message, "%s,%i", "hello", counter);
	// memcpy(datagram.destination, RECEIVER, ADDR_LENGTH);
	// datagram.type = 's'; // can be anything, but 's' for 'sensor'

	// // Send packet
	// layer2.writeData(datagram, msglen + DATAGRAM_HEADER);
	// counter++;
	// Serial.println((char *)datagram.message);
}

void serverloop()
{
	if (wperfTimer.isExpired())
	{
		wperfTimer.repeat();
#ifdef ARDUINO_NUCLEO_F303K8
		Serial.println(packetCount, DEC);
#elif defined(ARDUINO_NodeMCU_32S)
		Serial.printf("Total bytes : %d\tTotal packets : %d\tBitrate : %.0f bps", totalBytes, packetCount, totalBytes * 8.0e3f / MEASURE_INTERVAL_ms);
		if (packetCount > 0)
		{
			Serial.printf("\tAverage RSSI : %.2f\tAverage SNR : %.2f\r\n", averageRssi / packetCount, averageSNR / packetCount);
		}
		else
		{
			Serial.println();
		}
#endif
		packetCount = 0;
		averageRssi = 0;
		averageSNR = 0;
		totalBytes = 0;
	}

	//Layer1 actions
	if (layer1->receive() > 0)
	{
		BufferEntry entry = layer1->rxBuffer->read();
		totalBytes += entry.length;
		packetCount++;
		averageRssi += layer1->getRSSI();
		averageSNR += layer1->getSNR();
	}

	//Layer2 actions
	// layer2.daemon();
}

void loop()
{
#ifdef ARDUINO_NUCLEO_F303K8
	clientloop();
	//serverloop();
#elif defined(ARDUINO_NodeMCU_32S)
	serverloop();
	//clientloop();
#endif
}