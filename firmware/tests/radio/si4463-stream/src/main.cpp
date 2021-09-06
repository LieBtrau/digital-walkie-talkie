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

#include <Si446x.h>

#define CHANNEL 0
#define MAX_PACKET_SIZE 10
#define TIMEOUT 1000

#define PACKET_NONE 0
#define PACKET_OK 1
#define PACKET_INVALID 2
const int MODE_SELECT_PIN = 27;
bool isClient = false;
Si446x si4463;

typedef struct
{
	uint8_t ready;
	uint32_t timestamp;
	int16_t rssi;
	uint8_t length;
	byte buffer[MAX_PACKET_SIZE];
} pingInfo_t;

static volatile pingInfo_t pingInfo;

void SI446X_CB_RXCOMPLETE(byte length)
{
	if (length > MAX_PACKET_SIZE)
		length = MAX_PACKET_SIZE;

	pingInfo.ready = PACKET_OK;
	pingInfo.timestamp = millis();
	pingInfo.rssi = si4463.getLatchedRSSI();
	pingInfo.length = length;

	si4463.read((byte*)pingInfo.buffer, length);

	// Radio will now be in idle mode
}

void SI446X_CB_RXINVALID(int16_t rssi)
{
	pingInfo.ready = PACKET_INVALID;
	pingInfo.rssi = rssi;
}

void setup()
{
	Serial.begin(115200);
	while (!Serial)
		;

	pinMode(LED_BUILTIN, OUTPUT); // LED
	pinMode(MODE_SELECT_PIN, INPUT_PULLUP);
	isClient = digitalRead(MODE_SELECT_PIN) == HIGH ? true : false;

	// Start up
	si4463.init();
	si4463.setTxPower(SI446X_MAX_TX_POWER);

	// Put into receive mode
	si4463.RX(CHANNEL);
	si4463.onReceive(SI446X_CB_RXCOMPLETE);
}

void clientloop()
{
	static uint8_t counter;
	static uint32_t sent;
	static uint32_t replies;
	static uint32_t timeouts;
	static uint32_t invalids;

	// Make data
	byte data[MAX_PACKET_SIZE] = {0};
	sprintf_P((char*)data, PSTR("test %hhu"), counter);
	counter++;

	Serial.print(F("Client : Sending data"));
	Serial.println((char*)data);

	uint32_t startTime = millis();

	// Send the data
	si4463.TX(data, sizeof(data), CHANNEL, SI446X_STATE_RX);
	sent++;

	// Put into receive mode
	Serial.println(F("Data sent, waiting for reply..."));

	uint8_t success;

	// Wait for reply with timeout
	uint32_t sendStartTime = millis();
	while (1)
	{
		success = pingInfo.ready;
		if (success != PACKET_NONE)
			break;
		else if (millis() - sendStartTime > TIMEOUT) // Timeout // TODO typecast to uint16_t
			break;
	}

	pingInfo.ready = PACKET_NONE;

	if (success == PACKET_NONE)
	{
		Serial.println(F("Ping timed out"));
		timeouts++;
	}
	else if (success == PACKET_INVALID)
	{
		Serial.print(F("Invalid packet! Signal: "));
		Serial.print(pingInfo.rssi);
		Serial.println(F("dBm"));
		invalids++;
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
		Serial.write((uint8_t *)pingInfo.buffer, sizeof(pingInfo.buffer));
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

	delay(100);
}

void serverloop()
{
	static uint32_t pings;
	static uint32_t invalids;

	Serial.println(F("Server: Waiting for ping..."));

	// Wait for data
	while (pingInfo.ready == PACKET_NONE)
		;

	if (pingInfo.ready != PACKET_OK)
	{
		invalids++;
		pingInfo.ready = PACKET_NONE;
		Serial.print(F("Invalid packet! Signal: "));
		Serial.print(pingInfo.rssi);
		Serial.println(F("dBm"));
		si4463.RX(CHANNEL);
	}
	else
	{
		pings++;
		pingInfo.ready = PACKET_NONE;

		Serial.println(F("Got ping, sending reply..."));
		delay(500);

		// Send back the data, once the transmission has completed go into receive mode
		si4463.TX((uint8_t *)pingInfo.buffer, pingInfo.length, CHANNEL, SI446X_STATE_RX);

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
		Serial.write((uint8_t *)pingInfo.buffer, sizeof(pingInfo.buffer));
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