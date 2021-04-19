#include <Arduino.h>
#include <RadioLib.h>

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
Si446x radio = new Module(5, 4, 16);

#define CHANNEL 0
#define MAX_PACKET_SIZE 10
#define TIMEOUT 1000

#define PACKET_NONE 0
#define PACKET_OK 1
#define PACKET_INVALID 2
const int MODE_SELECT_PIN = 25;
bool isClient = false;

typedef struct
{
	uint8_t ready;
	uint32_t timestamp;
	int16_t rssi;
	uint8_t length;
	uint8_t buffer[MAX_PACKET_SIZE];
} pingInfo_t;

static volatile pingInfo_t pingInfo;

void setup()
{
	Serial.begin(115200);
	delay(1000);
	Serial.printf("Build %s\r\n", __TIMESTAMP__);

	pinMode(LED_BUILTIN, OUTPUT); // LED
	pinMode(MODE_SELECT_PIN, INPUT_PULLUP);
	isClient = digitalRead(MODE_SELECT_PIN) == HIGH ? true : false;

	// initialize Si4432 with default settings
	Serial.print(F("[Si4463] Initializing ... "));
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
	radio.setTxPower(SI446X_MAX_TX_POWER);
}

void clientloop()
{
	static uint8_t counter;
	static uint32_t sent;
	static uint32_t replies;
	static uint32_t timeouts;
	static uint32_t invalids;
	uint8_t success;

	// Make data
	char data[MAX_PACKET_SIZE] = {0};
	sprintf_P(data, PSTR("test %hhu"), counter);
	counter++;

	Serial.print(F("Client : Sending data"));
	Serial.println(data);

	uint32_t startTime = millis();

	// Send the data
	if (radio.transmit((byte *)data, sizeof(data), 0) != ERR_NONE)
	{
		Serial.println("TX error");
	}
	sent++;

	// Put into receive mode
	Serial.println(F("Data sent, waiting for reply..."));

	// Wait for reply with timeout
	for (int i = 0; i < 10; i++)
	{
		success = radio.receive((byte *)data, 0);
		if (success == ERR_NONE)
			break;
	}
	if (success != ERR_NONE)
	{
		Serial.println(F("Ping timed out"));
		timeouts++;
	}
	else
	{
		// If success toggle LED and send ping time over UART
		uint16_t totalTime = pingInfo.timestamp - startTime;

		static uint8_t ledState;
		digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
		ledState = !ledState;

		replies++;

		Serial.print(F("Ping time: "));
		Serial.print(totalTime);
		Serial.println(F("ms"));

		Serial.print(F("Signal strength: "));
		Serial.print(pingInfo.rssi);
		Serial.println(F("dBm"));

		// Print out ping contents
		Serial.print(F("Data from server: "));
		Serial.write((uint8_t *)data, radio.getPacketLength());
		Serial.println();
	}

	Serial.print(F("Totals: "));
	Serial.print(sent);
	Serial.print(F(" Sent, "));
	Serial.print(replies);
	Serial.print(F(" Replies, "));
	Serial.print(timeouts);
	Serial.print(F(" Timeouts, "));
	Serial.print(invalids);
	Serial.println(F(" Invalid"));
	Serial.println(F("------"));
	delay(1000);
}

void serverloop()
{
	static uint32_t pings;
	static uint32_t invalids;
	byte data[100];

	Serial.println(F("Server: Waiting for ping..."));

	uint8_t success = radio.receive(data, 0);

	if (success != ERR_NONE)
	{
		invalids++;
		pingInfo.ready = PACKET_NONE;
		Serial.print(F("Invalid packet! Signal: "));
		Serial.print(pingInfo.rssi);
		Serial.println(F("dBm"));
	}
	else
	{
		pings++;
		pingInfo.ready = PACKET_NONE;

		Serial.println(F("Got ping, sending reply..."));
		delay(500);

		// Send back the data, once the transmission has completed go into receive mode
		radio.transmit(data, radio.getPacketLength(), 0);

		Serial.println(F("Reply sent"));

		// Toggle LED
		static uint8_t ledState;
		digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
		ledState = !ledState;

		Serial.print(F("Signal strength: "));
		Serial.print(pingInfo.rssi);
		Serial.println(F("dBm"));

		// Print out ping contents
		Serial.print(F("Data from server: "));
		Serial.write(data, radio.getPacketLength());
		Serial.println();
	}

	Serial.print(F("Totals: "));
	Serial.print(pings);
	Serial.print(F("Pings, "));
	Serial.print(invalids);
	Serial.println(F("Invalid"));
	Serial.println(F("------"));
}

void loop()
{
	if (!isClient)
	{
		serverloop();
	}
	else
	{
		clientloop();
	}
}