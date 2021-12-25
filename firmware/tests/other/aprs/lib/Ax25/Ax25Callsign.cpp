#include "Ax25Callsign.h"

Ax25Callsign::Ax25Callsign(std::string name, byte ssid) : Ax25Callsign()
{
    assert(!name.empty() && name.length() <= MAX_CALLSIGN_LEN);
    _name = name;
    assert(ssid <= 0xF);
    _ssid = ssid;
}

Ax25Callsign::Ax25Callsign()
{
}

Ax25Callsign::~Ax25Callsign()
{
}

Ax25Callsign::Ax25Callsign(const Ax25Callsign &obj)
{
    _name = obj._name;
    _ssid = obj._ssid;
}

Ax25Callsign &Ax25Callsign::operator=(const Ax25Callsign &obj)
{
    _name = obj._name;
    _ssid = obj._ssid;
    return *this;
}

std::string Ax25Callsign::getName() const
{
    return _name;
}

byte Ax25Callsign::getSsid()
{
    return _ssid;
}