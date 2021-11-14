#pragma once

#include "Arduino.h"
#include "CircularBuffer.h"

class KissTnc : public Stream
{
private:
    static const word MAX_PAYLOAD_LEN = 500;
    static const byte FEND = 0xC0;
    static const byte FESC = 0xDB;
    static const byte TFEND = 0xDC;
    static const byte TFESC = 0xDD;

    bool addRxData(byte c);
    void handlePacketReady();
    void error(int, const char *, int);

    /* data */
    Stream *_host;
    CircularBuffer<byte, MAX_PAYLOAD_LEN> _txSinglePacketBuffer; //!< radio -> host
    CircularBuffer<byte, MAX_PAYLOAD_LEN> _rxSinglePacketBuffer; //!< host -> radio
    enum
    {
        WAIT_FOR_FEND,
        WAIT_FOR_COMMAND,
        WAIT_FOR_DATA,
        WAIT_FOR_TRANSPOSE
    } _rxDataState = WAIT_FOR_FEND;
    void (*_onExitKiss)() = nullptr;
    void (*_onError)(int, const char *, int) = nullptr;
    void (*_onDataReceived)(int) = nullptr;
    void (*_onTxDelayUpdate)(byte) = nullptr;
    void (*_onPersistanceUpdate)(byte) = nullptr;
    void (*_onSlotTimeUpdate)(byte) = nullptr;
    void (*_onTxTailUpdate)(byte) = nullptr;
    void (*_onFullDuplexUpdate)(byte) = nullptr;
    void (*_onSetHardwareReceived)(int) = nullptr;
    typedef enum
    {
        CMD_DATAFRAME = 0,
        CMD_TXDELAY = 1,
        CMD_PERSISTENCE = 2,
        CMD_SLOTTIME = 3,
        CMD_TXTAIL = 4,
        CMD_FULLDUPLEX = 5,
        CMD_SETHARDWARE = 6,
        CMD_INVALID,
        CMD_RETURN = 0xFF
    } COMMANDS;
    COMMANDS _command = CMD_INVALID;
    byte _txDelay, _p, _slotTime, _txTail, _fullDuplex; //!< Radio parameter cache

public:
    KissTnc(Stream *host);
    ~KissTnc();
    // from Print
    virtual size_t write(uint8_t c);
    virtual size_t write(const uint8_t *buffer, size_t size);

    // from Stream
    virtual int available();
    virtual int read();
    virtual int peek();
    virtual void flush();

    void beginPacket();
    void endPacket();
    void loop();

    //Callback
    void onExitKiss(void (*callback)());
    void onError(void (*callback)(int, const char *, int));
    void onDataReceived(void (*callback)(int));
    void onSetHardwareReceived(void (*callback)(int));
    void onTxDelayUpdate(void (*callback)(byte txdelay));
    void onPersistanceUpdate(void (*callback)(byte p));
    void onSlotTimeUpdate(void (*callback)(byte slotTime));
    void onTxTailUpdate(void (*callback)(byte txTail));
    void onFullDuplexUpdate(void (*callback)(byte fullDuplex));
};
