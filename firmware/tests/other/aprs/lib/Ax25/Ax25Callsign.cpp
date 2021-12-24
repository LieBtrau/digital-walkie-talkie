#include "Ax25Callsign.h"

#define DEBUG

#ifdef DEBUG

static int error(int err, const char *file, int line)
{
    printf("Error: %s (%d) at %s:%d\n", "" /* strerror[err] */, err, file, line);
    // while (1);
    return err;
}
#endif

Ax25Callsign::Ax25Callsign(std::string name, byte ssid) : Ax25Callsign()
{
    if (name.length() > MAX_CALLSIGN_LEN)
    {
#ifdef DEBUG
        error(-1, __FILE__, __LINE__);
#endif
    }

    _name = name;
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