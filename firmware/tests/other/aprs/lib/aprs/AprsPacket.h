#pragma once
#include "Arduino.h"

/**
 * @brief Base class describing APRS-packet format
 * APRS is a protocol that uses printable (and thus human readable) ASCII characters 
 *
 */
class AprsPacket
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
    bool setComment(const byte *buffer, byte len);

public:
    typedef enum
    {
        PKT_UNSUPPORTED,
        PKT_TEXT,
        PKT_LOCATION
    } PACKET_TYPE;
    static const byte CONTROL = 0x03;
    static const byte PROTOCOL_ID = 0xF0;
    AprsPacket(byte dti);
    virtual ~AprsPacket();
    PACKET_TYPE getPacketType();
    static AprsPacket *decode(const byte *buffer, size_t len);
    virtual const std::string encode() const = 0;
};