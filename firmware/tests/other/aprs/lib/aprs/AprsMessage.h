#pragma once

#include "AprsPacket.h"

/**
 * @brief APRS packet containing human readable info
 * APRS messages, bulletins and announcements are packets containing free
 * format text strings, and are intended to convey human-readable information.
 * A message is intended for reception by a single specified recipient, and an
 * acknowledgement is usually expected. Bulletins and announcements are
 * intended for reception by multiple recipients, and are not acknowledged.
 */
class AprsMessage : public AprsPacket
{

public:
    typedef enum
    {
        MSG_NOT_DEFINED,
        MSG_PLAIN,
        MSG_ACK,
        MSG_REJECT
    } MESSAGE_TYPE;
    AprsMessage(const byte *buffer, size_t len);
    AprsMessage(const char *text, int msgNr);
    ~AprsMessage();
    const std::string getAddressee() const;
    const std::string getMessage() const;
    int getMessageId() const;
    MESSAGE_TYPE getMessageType() const;
    bool isAckRequired() const;
    const char *encode() const;
    bool setMessageText(const std::string text, int msgNr = 0);
    bool setAddressee(const std::string addressee);

private:
    std::string _addressee;
    std::string _messageText; //!< Text only.  No '/0' allowed.
    int _messageNo = 0;
};