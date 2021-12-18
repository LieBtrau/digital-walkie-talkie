#include "Ax25Callsign.h"

#define DEBUG

#ifdef DEBUG
#include "stdio.h"
static int error(int err, const char *file, int line)
{
    printf("Error: %s (%d) at %s:%d\n", "" /* strerror[err] */, err, file, line);
    // while (1);
    return err;
}
#endif

Ax25Callsign::Ax25Callsign(const char *name, byte ssid) : Ax25Callsign()
{
    if (strlen(name) > MAX_CALLSIGN_LEN)
    {
#ifdef DEBUG
        error(-1, __FILE__, __LINE__);
#endif
    }

    strcpy(_name, name);
    _ssid = ssid;
}

Ax25Callsign::Ax25Callsign()
{
    _name = new char[MAX_CALLSIGN_LEN + 1];
}

Ax25Callsign::~Ax25Callsign()
{
    delete[] _name;
}

Ax25Callsign::Ax25Callsign(const Ax25Callsign &obj)
{
    _name = new char[MAX_CALLSIGN_LEN + 1];
    strcpy(_name, obj._name);
    _ssid = obj._ssid;
}

Ax25Callsign &Ax25Callsign::operator=(const Ax25Callsign &obj)
{
    strcpy(_name, obj._name);
    _ssid = obj._ssid;
    return *this;
}

const char *Ax25Callsign::getName()
{
    return _name;
}

byte Ax25Callsign::getSsid()
{
    return _ssid;
}