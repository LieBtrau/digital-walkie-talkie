#pragma once

#include "KissTnc.h"
#include "Ax25Frame.h"
#include "Ax25Callsign.h"

class Ax25Client
{
public:
    Ax25Client(KissTnc &tnc, const Ax25Callsign &callsign);
    ~Ax25Client();
    void setDestinationAddress(const Ax25Callsign &callsign);
    bool addDigipeaterAddress(const Ax25Callsign &callsign);
    bool sendFrame(byte control, byte protocolId, const byte *info_field, size_t info_len);
    bool sendFrame(byte control, byte protocolId, const std::string& info_field);
    void setRxFrameCallback(void (*callback)(const Ax25Callsign &destination, const Ax25Callsign &sender, const byte *info_field, size_t info_length));
    const std::string getMyCallsign();
    void loop();

private:
    KissTnc *_tnc = nullptr;
    Ax25Callsign _destinationAddress;
    Ax25Callsign _sourceAddress;
    std::array<Ax25Callsign, 8> _digipeaterList;
    size_t _digipeaterCount = 0;
    void (*_rxFrameCallback)(const Ax25Callsign &destination, const Ax25Callsign &sender, const byte *info_field, size_t info_length) = nullptr;
    friend void dataReceivedHandler(int length);
};