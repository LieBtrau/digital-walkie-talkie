#include <Arduino.h>
#include "BluetoothSerial.h"
#include "KissTnc.h"
#include "Ax25Client.h"
#include "AprsMessage.h"
#include "AprsPositionReport.h"

BluetoothSerial SerialBT;
KissTnc kisstnc(&SerialBT);
Ax25Client ax25client(&kisstnc, Ax25Callsign("NOCALL", 0));

void exitKissHandler()
{
	Serial.println("Host wants to exit kiss-mode.");
}

void errorHandler(int err, const char *file, int line)
{
	Serial.printf("Error: (%d) at %s:%d\n\r", err, file, line);
}

void setHardwareHandler(int length)
{
	byte rxDataBuffer[500];
	kisstnc.readBytes(rxDataBuffer, length);
	Serial.printf("Set hardware: %d bytes\r\n", length);
}

void txDelayUpdateHandler(byte txDelay)
{
	Serial.printf("TxDelay: %d\r\n", txDelay);
}

void persistenceUpdateHandler(byte persistence)
{
	Serial.printf("Persistence: %d\r\n", persistence);
}

void slotTimeUpdateHandler(byte slotTime)
{
	Serial.printf("Slot time: %d\r\n", slotTime);
}

void txTailUpdateHandler(byte txTail)
{
	Serial.printf("TxTail: %d\r\n", txTail);
}

void fullDuplexUpdateHandler(byte fullDuplex)
{
	Serial.printf("Full duplex: %d\r\n", fullDuplex);
}

void ax25receivedHandler(const Ax25Callsign &destination, const Ax25Callsign &sender, const byte *info_field, size_t info_length)
{
	Serial.printf("\r\nDestination: %s\r\nSender: %s\r\n", destination.getName(), sender.getName());

	AprsPacket *aprsPacket = AprsPacket::decode(info_field, info_length);
	if (aprsPacket->getPacketType() == AprsPacket::PKT_TEXT)
	{
		AprsMessage *aprsMsg = (AprsMessage *)aprsPacket;
		Serial.printf("Addressee:\"%s\"\r\nMessage text: \"%s\"\r\nMessage ID: %d\r\n",
					  aprsMsg->getAddressee(),
					  aprsMsg->getMessage(),
					  aprsMsg->getMessageId());
		delete aprsMsg;
	}

	if (aprsPacket->getPacketType() == AprsPacket::PKT_LOCATION)
	{
		AprsPositionReport *aprsPos = (AprsPositionReport *)aprsPacket;
		Serial.printf("Latitude: \"%s\"\r\nLongitude: \"%s\"\r\nSymbolTableId=%d\r\nSymbolCode=%d\r\n",
					  aprsPos->getLatitude(),
					  aprsPos->getLongitude(),
					  aprsPos->getSymbolTableId(),
					  aprsPos->getSymbolCode());
		delete aprsPos;
	}

}

void setup()
{
	Serial.begin(115200);
	SerialBT.begin("ESP32 KISS TNC");
	kisstnc.onExitKiss(exitKissHandler);
	kisstnc.onError(errorHandler);
	kisstnc.onSetHardwareReceived(setHardwareHandler);
	kisstnc.onTxDelayUpdate(txDelayUpdateHandler);
	kisstnc.onPersistanceUpdate(persistenceUpdateHandler);
	kisstnc.onSlotTimeUpdate(slotTimeUpdateHandler);
	kisstnc.onTxTailUpdate(txTailUpdateHandler);
	kisstnc.onFullDuplexUpdate(fullDuplexUpdateHandler);
	ax25client.setRxFrameCallback(ax25receivedHandler);
	delay(1000);
	Serial.printf("Build %s\r\n", __TIMESTAMP__); // timestamp only gets updated when this file is being recompiled.
	do
	{
		Serial.println("Waiting for bluetooth connection...");
	} while (!SerialBT.connected(10000));
	pinMode(BUILTIN_LED, OUTPUT);
}

void loop()
{
	ax25client.loop();
}