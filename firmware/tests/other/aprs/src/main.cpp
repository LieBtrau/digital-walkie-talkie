#include <Arduino.h>
#include "Ax25Frame.h"
#include "AprsMessage.h"
#include "AprsPositionReport.h"

void setup()
{
	Serial.begin(115200);
	Serial.println();

	byte ax25bufferMsg[] = {0x82, 0xA0, 0x88, 0xA4, 0x62, 0x6C, 0x60, 0x9C, 0x60, 0x86, 0x82, 0x98, 0x98, 0x61, 0x03, 0xF0, 0x3A, 0x4E,
							0x30, 0x43, 0x41, 0x4C, 0x4C, 0x20, 0x20, 0x20, 0x3A, 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x77, 0x6F, 0x72,
							0x6C, 0x64, 0x7B, 0x35};
	byte ax25bufferPos[] = {0x82, 0xa0, 0x88, 0xa4, 0x62, 0x6c, 0xe0, 0x9c, 0x60, 0x86, 0x82, 0x98, 0x98, 0x61, 0x03, 0xf0, 0x3d, 0x30,
							0x31, 0x30, 0x30, 0x2e, 0x30, 0x30, 0x4e, 0x5c, 0x30, 0x30, 0x32, 0x30, 0x30, 0x2e, 0x30, 0x30, 0x45, 0x29,
							0x20, 0x68, 0x74, 0x74, 0x70, 0x73, 0x3a, 0x2f, 0x2f, 0x61, 0x70, 0x72, 0x73, 0x64, 0x72, 0x6f, 0x69, 0x64,
							0x2e, 0x6f, 0x72, 0x67, 0x2f};
	AX25Frame rxframe((const byte *)ax25bufferMsg, sizeof(ax25bufferMsg));
	AprsPacket *aprsPacket = AprsPacket::decode(rxframe.getInfoField(), rxframe.getInfoLength());
	if (aprsPacket->getPacketType() == AprsPacket::PKT_TEXT)
	{
		AprsMessage *aprsMsg = (AprsMessage *)aprsPacket;
		Serial.printf("Addressee:\"%s\"\r\nMessage text: \"%s\"\r\nMessage ID: %d\r\n",
					  aprsMsg->getAddressee(),
					  aprsMsg->getMessage().c_str(),
					  aprsMsg->getMessageId());
		const char *outBuffer = aprsMsg->encode().c_str();
		Serial.printf("Output composed APRS-message: %s", outBuffer);
		std::array<Ax25Callsign, 8> digipeaterList;
		AX25Frame ax25out(Ax25Callsign("APDR16", 0), Ax25Callsign("N0CALL", 0), digipeaterList, AprsPacket::CONTROL, AprsPacket::PROTOCOL_ID, (const byte*)outBuffer, strlen(outBuffer));
		size_t bufferLen;
		byte *ax25Buffer = ax25out.encode(bufferLen);
		for (int i = 0; i < bufferLen; i++)
		{
			Serial.printf("0x%02x, ", ax25Buffer[i]);
		}
		delete[] ax25Buffer;
	}

	if (aprsPacket->getPacketType() == AprsPacket::PKT_LOCATION)
	{
		AprsPositionReport *aprsPos = (AprsPositionReport *)aprsPacket;
		Serial.printf("Latitude: \"%s\"\r\nLongitude: \"%s\"\r\nSymbolTableId=%d\r\nSymbolCode=%d\r\n",
					  aprsPos->getLatitude(),
					  aprsPos->getLongitude(),
					  aprsPos->getSymbolTableId(),
					  aprsPos->getSymbolCode());
	}
	Serial.println("done");
}

void loop()
{
}