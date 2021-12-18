#pragma once

#include "libAprs.h"

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
private:
    char *addressee = nullptr;
    char *messageText = nullptr;//!< Text only.  No '/0' allowed.
    int messageNo = 0;

public:
    AprsMessage(const byte *buffer, size_t len);
    ~AprsMessage();
    const char *getAddressee();
    const char *getMessage();
    int getMessageId();
    bool setMessageText(const char *buffer, int msgNr = 0);
    char* encode();
};