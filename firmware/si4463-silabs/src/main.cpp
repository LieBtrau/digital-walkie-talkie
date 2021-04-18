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

void setup()
{
	Serial.begin(115200);
	delay(1000);
	Serial.printf("Build %s\r\n", __TIMESTAMP__);

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

void loop()
{
	// put your main code here, to run repeatedly:
	byte data[100];
	int err = radio.receive(data, 0);
	if (err == ERR_NONE)
	{
		Serial.printf("packet length: %d\tpacket data: %s\r\n", radio.getPacketLength(), data);
	}
	else
	{
		Serial.printf("Error: %d\r\n", err);
	}
}