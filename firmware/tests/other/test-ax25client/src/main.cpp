#include <Arduino.h>
#include "BluetoothSerial.h"
#include "KissTnc.h"
#include "Ax25Client.h"
#include "AprsClient.h"
#include "AprsMessage.h"
#include "AsyncDelay.h"

BluetoothSerial SerialBT;
KissTnc kisstnc(SerialBT);
Ax25Client ax25client(kisstnc, Ax25Callsign("I0CALL", 0));
AprsClient aprsClient(ax25client);
const int PTT_BUTTON = 27;
AsyncDelay superFrameTimer;
int messageId = 0;

void errorHandler(int err, const char *file, int line)
{
	Serial.printf("Error: (%d) at %s:%d\n\r", err, file, line);
}

void ax25receivedHandler(const Ax25Callsign &destination, const Ax25Callsign &sender, const byte *info_field, size_t info_length)
{
	aprsClient.receiveFrame(destination, sender, info_field, info_length);
}

void aprsMessageReceivedHandler(const char *addressee, const char *message)
{
	Serial.printf("Hey! Message received.\r\nAddressee:\"%s\"\r\nMessage text: \"%s\"\r\n", addressee, message);
}

void aprsAckReceivedHandler(int id)
{
	Serial.printf("Ack received: %d %d", messageId, id);
}

void setup()
{
	Serial.begin(115200);
	SerialBT.begin("ESP32 KISS TNC");
	kisstnc.onError(errorHandler);
	ax25client.setRxFrameCallback(ax25receivedHandler);
	ax25client.setDestinationAddress(Ax25Callsign("APDR16", 0));
	aprsClient.setMessageReceivedCallback(aprsMessageReceivedHandler);
	aprsClient.setAckReceivedCallback(aprsAckReceivedHandler);
	delay(1000);
	Serial.printf("Build %s\r\n", __TIMESTAMP__); // timestamp only gets updated when this file is being recompiled.
	do
	{
		Serial.println("Waiting for bluetooth connection...");
	} while (!SerialBT.connected(10000));
	Serial.println("Bluetooth connected");
	superFrameTimer.start(1000, AsyncDelay::MILLIS);
	pinMode(PTT_BUTTON, INPUT_PULLUP);
	pinMode(BUILTIN_LED, OUTPUT);
}

void loop()
{
	aprsClient.loop();
	if (superFrameTimer.isExpired())
	{
		digitalWrite(BUILTIN_LED, digitalRead(BUILTIN_LED) == HIGH ? LOW : HIGH);
		if (digitalRead(PTT_BUTTON) == LOW)
		{
			Ax25Callsign peer("N0CALL", 0);
			messageId = aprsClient.sendMessage(peer, "Test", true);
			superFrameTimer.restart();
		}
		superFrameTimer.restart();
	}
}