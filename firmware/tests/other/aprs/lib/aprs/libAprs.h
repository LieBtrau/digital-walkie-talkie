#pragma once
#include "Arduino.h"

/**
 * @brief Base class describing APRS-packet format
 *
 */
class libAprs
{
private:
    byte *dataExtension = nullptr;
    size_t dataExtensionLen = 0;

protected:
    typedef enum
    {
        MESSAGE = ':',
        // 4 types of Position/DF (=direction finding) report
        POS_NO_TIME_NO_MSG = '!',
        POS_NO_TIME_WITH_MSG = '=',
        POS_WITH_TIME_WITH_MSG = '@',
        POS_WITH_TIME_NO_MSG = '/'
    } DTI;
    DTI dataTypeId; //!< APRS Data Type Identifier (see APRS101, chapter 5 APRS DATA IN THE AX.25 INFORMATION FIELD)
    byte *comment = nullptr;
    size_t commentLen = 0;
    bool hasAprsExtension(const byte *buffer);
    void setComment(const byte* buffer, byte len);

public:
    typedef enum
    {
        PKT_UNSUPPORTED,
        PKT_TEXT,
        PKT_LOCATION
    } PACKET_TYPE;
    PACKET_TYPE getPacketType();
    static libAprs *decode(byte *buffer, size_t len);
    libAprs(byte dti);
    ~libAprs();
};

/**
 * @brief Position and DF reports
 * According to chapter 8 POSITION AND DF REPORT DATA FORMATS of APRS 101
 */
class AprsPositionReport : public libAprs
{
private:
    char *latitude = nullptr;
    char *longitude = nullptr;
    byte symbolTableId=0;
    byte symbolCode=0;

public:
    AprsPositionReport(const byte *buffer, size_t len);
    ~AprsPositionReport();
    const char* getLatitude();
    const char* getLongitude();
    byte getSymbolTableId();
    byte getSymbolCode();
};

/**
 * @brief APRS packet containing human readable info
 * APRS messages, bulletins and announcements are packets containing free
 * format text strings, and are intended to convey human-readable information.
 * A message is intended for reception by a single specified recipient, and an
 * acknowledgement is usually expected. Bulletins and announcements are
 * intended for reception by multiple recipients, and are not acknowledged.
 */
class AprsMessage : public libAprs
{
private:
    char *addressee = nullptr;
    char *messageText = nullptr;
    int messageId = 0;

public:
    AprsMessage(const byte *buffer, size_t len);
    ~AprsMessage();
    const char *getAddressee();
    const char *getMessage();
    int getMessageId();
};