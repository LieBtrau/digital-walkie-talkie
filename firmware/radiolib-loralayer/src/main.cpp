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
Layer1Class Layer1(&radio, 0,7);
//Layer2 layer2(&Layer1);
BufferEntry entry;
const int MEASURE_INTERVAL_ms = 10000;
int packetCount = 0;
float averageRssi = 0;
float averageSNR = 0;
int totalBytes = 0;

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
	wperfTimer.start(MEASURE_INTERVAL_ms, AsyncDelay::MILLIS);
	//Layer1 actions
	byte s[10] = {0};
	memcpy(entry.data, s, sizeof(s));
	entry.length = sizeof(s);
}

void clientloop()
{

	Layer1.txBuffer->write(entry);
	Layer1.transmit();
	delay(20);
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
		Serial.printf("Total bytes : %d\tTotal packets : %d\tBitrate : %.0f bps", totalBytes, packetCount, totalBytes * 8.0e3f / MEASURE_INTERVAL_ms);
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
	}

	//Layer1 actions
	if (Layer1.receive() > 0)
	{
		BufferEntry entry = Layer1.rxBuffer->read();
		//Serial.println(entry.data);
		totalBytes += entry.length;
		packetCount++;
		averageRssi += radio.getRSSI();
		averageSNR += radio.getSNR();
	}

	//Layer2 actions
	// layer2.daemon();
}

void loop()
{
#ifdef ARDUINO_NUCLEO_F303K8
	clientloop();
#elif defined(ARDUINO_NodeMCU_32S)
	serverloop();
#endif
}