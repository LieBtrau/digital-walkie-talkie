#pragma once
#include "Arduino.h"

class Ax25Callsign
{
public:
	static const byte MAX_CALLSIGN_LEN = 6;
	Ax25Callsign(const char *name, byte ssid);
	Ax25Callsign();
	~Ax25Callsign();
	Ax25Callsign(const Ax25Callsign &obj);
	Ax25Callsign &operator=(const Ax25Callsign &obj);
	const char *getName() const;
	byte getSsid();

private:
	char *_name = nullptr; //!<<6 bytes name
	byte _ssid = 0;		   //!<<4 bit ssid
};