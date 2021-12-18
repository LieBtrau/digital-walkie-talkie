#pragma once
#include "Arduino.h"


// /**
//  * @brief Dealing with sending and receiving APRS-packets from a TNC-client
//  */
// class AprsClient
// {
// private:
//     void (*_read)(const byte* buffer, size_t len) = nullptr;
//     void (*_write)(const byte* buffer, size_t len) = nullptr;
//     char* myCallsign = nullptr;
//     byte _mySsid = 0;
//     char* destCallsign=nullptr;
//     byte destSsid = 0;

// public:
//     void setMyCallsign(const char* callsign, byte ssid);
//     void setDestination(const char* callsign, byte ssid);
//     /**
//      * @brief Callback for incoming AX.25 frames
//      * AX.25 frames coming from the TNC that need to be encoded.
//      * @param callback 
//      */
//     void read(void (*callback)(const byte* buffer, size_t len));
//     /**
//      * @brief Callback for outgoing AX.25 frames
//      * AX.25 frames that must be sent downto the TNC.
//      * @param callback 
//      */
//     void write(void (*callback)(const byte* buffer, size_t len));
//     ~AprsClient();
// };

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
    PACKET_TYPE getPacketType();
    static AprsPacket *decode(byte *buffer, size_t len);
    virtual char* encode() = 0;
    AprsPacket(byte dti);
    ~AprsPacket();
};