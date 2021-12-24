#pragma once
#include "Arduino.h"

class Ax25Callsign
{
public:
	static const byte MAX_CALLSIGN_LEN = 6;
	Ax25Callsign(std::string name, byte ssid);
	Ax25Callsign();
	~Ax25Callsign();
	Ax25Callsign(const Ax25Callsign &obj);
	Ax25Callsign &operator=(const Ax25Callsign &obj);
	std::string getName() const;
	byte getSsid();

private:
	std::string _name; 	//!<<6 bytes name
	byte _ssid = 0;		//!<<4 bit ssid
};