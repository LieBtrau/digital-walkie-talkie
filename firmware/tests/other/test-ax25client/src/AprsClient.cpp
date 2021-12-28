#include "AprsClient.h"
#include "AprsMessage.h"
#include "AprsPositionReport.h"

AprsClient::AprsClient(Ax25Client &ax25client) : _ax25Client(&ax25client)
{
}

AprsClient::~AprsClient()
{
}

/**
 * @brief Send a message to someone, with or without requesting an ACK
 * When sending a new message, attempts to resend a message that hasn't been ack'ed yet will be abandoned.
 * @param destination callsign of the addressee, no SSID
 * @param message message text
 * @param ackRequired true when ack required, else false.
 * @return int ID of the sent message, can later be matched with the ACK to confirm reception of a certain message.  Will be 0 when no ack is expected.
 */
int AprsClient::sendMessage(Ax25Callsign &destination, const char *message, bool ackRequired)
{
    AprsMessage aprsMessage(message, ackRequired ? ++_messageCounter : 0);
    aprsMessage.setAddressee(destination.getName().c_str());
    strcpy(_info_field,aprsMessage.encode());
    if (ackRequired)
    {
        _sendTrialCounter = MAX_TX_RETRIES;
        _resendTimer.start(30000, AsyncDelay::MILLIS);
        if (sendMessage())
        {
            return _messageCounter;
        }
    }
    else
    {
        _sendTrialCounter = 0;
        sendMessage();
    }
    return 0;
}

bool AprsClient::sendMessage()
{
    if (_info_field != nullptr)
    {
        return _ax25Client->sendFrame(AprsPacket::CONTROL, AprsPacket::PROTOCOL_ID, (const byte *)_info_field, strlen(_info_field));
    }
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

void AprsClient::setAckReceivedCallback(void (*callback)(int messageId))
{
    _ackReceivedCallback = callback;
}

void AprsClient::receiveFrame(const Ax25Callsign &destination, const Ax25Callsign &sender, const byte *info_field, size_t info_length)
{
    AprsMessage *aprsMsg;
    AprsPositionReport *aprsPos;
    // Serial.printf("\r\nDestination: %s\r\nSender: %s\r\n", destination.getName().c_str(), sender.getName().c_str());

    AprsPacket *aprsPacket = AprsPacket::decode(info_field, info_length);
    switch (aprsPacket->getPacketType())
    {
    case AprsPacket::PKT_TEXT:
        aprsMsg = (AprsMessage *)aprsPacket;
        // Serial.printf("Message id: %d\r\n", aprsMsg->getMessageId());
        if (_messageReceivedCallback != nullptr)
        {
            _messageReceivedCallback(aprsMsg->getAddressee().c_str(), aprsMsg->getMessage());
        }
        else
        {
            // Simply dump message on to serial output
            Serial.printf("Addressee:\"%s\"\r\nMessage text: \"%s\"\r\nMessage ID: %d\r\n",
                          aprsMsg->getAddressee().c_str(),
                          aprsMsg->getMessage(),
                          aprsMsg->getMessageId());
        }
        if (aprsMsg->getMessageType() == AprsMessage::MSG_ACK && aprsMsg->getMessageId() == _messageCounter)
        {
            // ACK received on last sent message
            _sendTrialCounter = 0;
            if (_ackReceivedCallback != nullptr)
            {
                _ackReceivedCallback(_messageCounter);
            }
        }
        if (aprsMsg->isAckRequired())
        {
            AprsMessage ackMsg((const char *)"ack", aprsMsg->getMessageId());
            ackMsg.setAddressee(sender.getName().c_str());
            const char *info_field = ackMsg.encode();
            char buffer[100];
            strcpy(buffer, info_field);
            _ax25Client->sendFrame(AprsPacket::CONTROL, AprsPacket::PROTOCOL_ID, (const byte *)buffer, strlen(info_field));
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

void AprsClient::loop()
{
    _ax25Client->loop();
    if (_resendTimer.isExpired() && _sendTrialCounter > 0)
    {
        _sendTrialCounter--;
        sendMessage();
        if (_sendTrialCounter > 0)
        {
            _resendTimer.restart();
        }
    }
}