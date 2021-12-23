#include "AprsClient.h"
#include "AprsMessage.h"
#include "AprsPositionReport.h"

AprsClient::AprsClient(Ax25Client &ax25client) : _ax25Client(&ax25client)
{
}

AprsClient::~AprsClient()
{
}

bool AprsClient::sendMessage(Ax25Callsign &destination, const char *message)
{
    return false;
}

bool AprsClient::sendLocation(float latitude, float longitude)
{
    return false;
}

void AprsClient::setLocationReceivedCallback(void (*callback)(const Ax25Callsign &sender, float latitude, float longitude))
{
    _locationReceivedCallback = callback;
}

void AprsClient::setMessageReceivedCallback(void (*callback)(const char *addressee, const char *message))
{
    _messageReceivedCallback = callback;
}

void AprsClient::receiveFrame(const Ax25Callsign &destination, const Ax25Callsign &sender, const byte *info_field, size_t info_length)
{
    AprsMessage *aprsMsg;
    AprsPositionReport *aprsPos;
    Serial.printf("\r\nDestination: %s\r\nSender: %s\r\n", destination.getName(), sender.getName());

    AprsPacket *aprsPacket = AprsPacket::decode(info_field, info_length);
    switch (aprsPacket->getPacketType())
    {
    case AprsPacket::PKT_TEXT:
        aprsMsg = (AprsMessage *)aprsPacket;
        if (_messageReceivedCallback != nullptr)
        {
            _messageReceivedCallback(aprsMsg->getAddressee(), aprsMsg->getMessage());
            if (aprsMsg->isAckRequired())
            {
                AprsMessage ackMsg((const char *)"ack", aprsMsg->getMessageId());
                ackMsg.setAddressee(sender.getName());
                char *info_field = ackMsg.encode();
                _ax25Client->sendFrame(AprsPacket::CONTROL, AprsPacket::PROTOCOL_ID, (const byte *)info_field, strlen(info_field));
                delete[] info_field;
            }
        }
        else
        {
            // Simply dump message on to serial output
            Serial.printf("Addressee:\"%s\"\r\nMessage text: \"%s\"\r\nMessage ID: %d\r\n",
                          aprsMsg->getAddressee(),
                          aprsMsg->getMessage(),
                          aprsMsg->getMessageId());
        }
        delete aprsMsg;
        break;
    case AprsPacket::PKT_LOCATION:
        aprsPos = (AprsPositionReport *)aprsPacket;
        if (_locationReceivedCallback != nullptr)
        {
        }
        else
        {
            Serial.printf("Latitude: \"%s\"\r\nLongitude: \"%s\"\r\nSymbolTableId=%d\r\nSymbolCode=%d\r\n",
                          aprsPos->getLatitude(),
                          aprsPos->getLongitude(),
                          aprsPos->getSymbolTableId(),
                          aprsPos->getSymbolCode());
        }
        delete aprsPos;
        break;
    default:
        break;
    }
}
