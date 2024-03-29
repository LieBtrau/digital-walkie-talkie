#include "AprsClient.h"
#include "AprsMessage.h"
#include "AprsPositionReport.h"

AprsClient::AprsClient(Ax25Client &ax25client, const AprsSymbol& symbol) : _ax25Client(&ax25client), _symbol(symbol)
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
int AprsClient::sendMessage(Ax25Callsign &destination, std::string &message, bool ackRequired)
{
    AprsMessage aprsMessage(message, ackRequired ? ++_messageCounter : 0);
    aprsMessage.setAddressee(destination.getName());
    _info_field = aprsMessage.encode();
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
    return _ax25Client->sendFrame(AprsPacket::CONTROL, AprsPacket::PROTOCOL_ID, _info_field);
}

bool AprsClient::sendLocation(float latitude, float longitude)
{
    AprsLocation location2(latitude, longitude);
    AprsSymbol symb1('/', 'E');
    std::string comment = std::string("hihi");
    AprsPositionReport aprspos1(location2, comment);
    aprspos1.setSymbol(symb1);
    _info_field = aprspos1.encode();
    Serial.printf("Send location : \"%s\"\r\n", _info_field.c_str());
    _sendTrialCounter = 0;
    return sendMessage();
}

void AprsClient::setLocationReceivedCallback(void (*callback)(const Ax25Callsign &sender, float latitude, float longitude))
{
    _locationReceivedCallback = callback;
}

void AprsClient::setMessageReceivedCallback(void (*callback)(const std::string &addressee, const std::string &message))
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
    for (int i = 0; i < info_length; i++)
    {
        Serial.printf("%c", info_field[i]);
    }

    AprsPacket *aprsPacket = AprsPacket::decode(info_field, info_length);
    switch (aprsPacket->getPacketType())
    {
    case AprsPacket::PKT_TEXT:
        aprsMsg = (AprsMessage *)aprsPacket;
        // Serial.printf("Message id: %d\r\n", aprsMsg->getMessageId());
        if (_messageReceivedCallback != nullptr)
        {
            _messageReceivedCallback(aprsMsg->getAddressee().c_str(), aprsMsg->getMessage().c_str());
        }
        else
        {
            // Simply dump message on to serial output
            Serial.printf("Addressee:\"%s\"\r\nMessage text: \"%s\"\r\nMessage ID: %d\r\n",
                          aprsMsg->getAddressee().c_str(),
                          aprsMsg->getMessage().c_str(),
                          aprsMsg->getMessageId());
        }
        if (aprsMsg->getAddressee() == _ax25Client->getMyCallsign())
        {
            // Message is meant for me.  Check what to do with it.
            if (aprsMsg->getMessageType() == AprsMessage::MSG_ACK && aprsMsg->getMessageId() == _messageCounter)
            {
                // ACK received on last sent message, so we can stop trying to resend it.
                _sendTrialCounter = 0;
                if (_ackReceivedCallback != nullptr)
                {
                    _ackReceivedCallback(_messageCounter);
                }
            }
            if (aprsMsg->isAckRequired())
            {
                AprsMessage ackMsg("ack", aprsMsg->getMessageId());
                ackMsg.setAddressee(sender.getName());
                _ax25Client->sendFrame(AprsPacket::CONTROL, AprsPacket::PROTOCOL_ID, ackMsg.encode());
            }
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
            AprsLocation loc = aprsPos->getPosition();
            AprsSymbol symb = aprsPos->getSymbol();
            Serial.printf("\r\nLatitude: \"%f\"\r\nLongitude: \"%f\"\r\nSymbolTableId=%s\r\nSymbolCode=%s\r\n",
                          loc.getLatitude(),
                          loc.getLongitude(),
                          symb.encodeTableId().c_str(),
                          symb.encodeSymbol().c_str());
            Serial.printf("\"%s\"\r\n", aprsPos->getComment().c_str());
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