#include "Ax25Client.h"

Ax25Client::Ax25Client(KissTnc *tnc, const Ax25Callsign *callsign) : _tnc(tnc), _sourceAddress(*callsign) {}

Ax25Client::~Ax25Client()
{
    delete[] _digipeaterList;
}

void Ax25Client::setDestinationAddress(const Ax25Callsign *callsign)
{
    _destinationAddress = *callsign;
}

void Ax25Client::setDigipeaterAddresses(const Ax25Callsign *list, size_t count)
{
    _digipeaterList = new Ax25Callsign[count];
    for (size_t i = 0; i < count; i++)
    {
        _digipeaterList[i] = list[i];
    }
}

bool Ax25Client::sendFrame(byte control, byte protocolId, const byte *info_field, size_t info_len)
{
    AX25Frame frame(_destinationAddress, _sourceAddress, _digipeaterList, _digipeaterCount, control, protocolId, info_field, info_len);
    size_t bufferlen=0;
    byte* buffer = frame.encode(bufferlen);
    // _tnc->beginPacket();
    // _tnc->write(buffer, bufferlen);
    // _tnc->endPacket();
    return true;
}

