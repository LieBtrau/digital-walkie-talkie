#pragma once

//#include "KissTnc.h"
#include "Ax25.h"
#include "Ax25Callsign.h"

class Ax25Client
{
public:
    //Ax25Client(KissTnc *tnc, const Ax25Callsign *callsign);
    ~Ax25Client();
    void setDestinationAddress(const Ax25Callsign *callsign);
    void setDigipeaterAddresses(const Ax25Callsign *list, size_t count);
    bool sendFrame(byte control, byte protocolId, const byte *info_field, size_t info_len);
    void setRxFrameCallback(void (*callback)(const Ax25Callsign *destination, const Ax25Callsign *sender, const byte *buffer, size_t len));
    void loop();

private:
    //KissTnc *_tnc;
    Ax25Callsign _destinationAddress;
    Ax25Callsign _sourceAddress;
    Ax25Callsign *_digipeaterList = nullptr;
    size_t _digipeaterCount = 0;
    void (*_rxFrameCallback)(const Ax25Callsign *destination, const Ax25Callsign *sender, const byte *buffer, size_t len) = nullptr;
};