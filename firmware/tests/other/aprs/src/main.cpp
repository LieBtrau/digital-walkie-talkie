#include <Arduino.h>
#include "Ax25.h"
#include "libAprs.h"

void setup()
{
	Serial.begin(115200);
	Serial.println();

	byte ax25buffer[] = {0x82, 0xA0, 0x88, 0xA4, 0x62, 0x6C, 0x60, 0x9C, 0x60, 0x86, 0x82, 0x98, 0x98, 0x61, 0x03, 0xF0, 0x3A, 0x4E,
						 0x30, 0x43, 0x41, 0x4C, 0x4C, 0x20, 0x20, 0x20, 0x3A, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72,
						 0x6C, 0x64, 0x7B, 0x35};
	AX25Frame rxframe((const byte *)ax25buffer, sizeof(ax25buffer));
	libAprs *aprsPacket = libAprs::decode(rxframe.info, rxframe.infoLen);
	if (aprsPacket->getPacketType() == libAprs::PKT_TEXT)
	{
		AprsMessage *aprsMsg = (AprsMessage *)aprsPacket;
		Serial.printf("Addressee:\"%s\"\r\nMessage text: \"%s\"\r\nMessage ID: %d\r\n",
					  aprsMsg->getAddressee(),
					  aprsMsg->getMessage(),
					  aprsMsg->getMessageId());
	}
	Serial.println("done");
}

void loop()
{
}