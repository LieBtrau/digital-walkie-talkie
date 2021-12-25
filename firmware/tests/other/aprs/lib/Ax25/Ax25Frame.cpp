// Based on : https://github.com/jgromes/RadioLib/blob/master/src/protocols/AX25/AX25.cpp

#include "Ax25Frame.h"

AX25Frame::AX25Frame(const Ax25Callsign &destCallsign, const Ax25Callsign &srcCallsign, const std::array<Ax25Callsign, 8> digipeaterList, byte control, byte protocolID, const byte *info, uint16_t infoLen)
{
	// destination callsign/SSID
	_addresses[DESTINATION] = destCallsign; // calls overloaded assignment operator
	// source callsign/SSID
	_addresses[SOURCE] = srcCallsign;
	// digipeater list
	int index = 0;
	for (auto digipeater : digipeaterList)
	{
		if (!digipeater.getName().empty())
		{
			_addresses[DIGIPEATER1 + index++];
		}
	}
	// control field
	_controlfield = control;
	// PID field
	_protocolID = protocolID;
	// info field
	_infoLen = infoLen;
	if (infoLen > 0)
	{
		this->_info = new byte[infoLen];
		memcpy(this->_info, info, infoLen);
	}
}

AX25Frame::AX25Frame(const Ax25Callsign &destCallsign, const Ax25Callsign &srcCallsign, const std::array<Ax25Callsign, 8> digipeaterList, byte control, byte protocolID, std::string info) : 
AX25Frame(destCallsign, srcCallsign, digipeaterList, control, protocolID, (const byte *)info.c_str(), info.length())
{
}

AX25Frame::AX25Frame(const byte *ax25data, size_t datalen)
{
	byte *ptrBuf = (byte *)ax25data;
	bool isLastAddress;
	// Decode callsigns : destination, source, digipeaters
	size_t addressCounter = 0;
	do
	{
		_addresses[addressCounter++] = decodeAddress(ptrBuf, isLastAddress);
		ptrBuf += 7;
	} while ((ptrBuf < ax25data + 70) && (!isLastAddress));
	_digipeaterCount = addressCounter - 2;
	// Decode control field.  For APRS, it's always 0x03.
	_controlfield = *(ptrBuf++);
	// Decode protocol ID.  For APRS, it's always 0xF0 (no layer 3 protocol)
	_protocolID = *(ptrBuf++);
	// Get the data from the information field
	_infoLen = ax25data + datalen - ptrBuf;
	_info = new byte[_infoLen + 1];
	memcpy(_info, ptrBuf, _infoLen);
	_info[_infoLen] = '\0';
}

AX25Frame::AX25Frame(const AX25Frame &frame)
{
	_digipeaterCount = frame._digipeaterCount;
	_addresses = frame._addresses;
	_controlfield = frame._controlfield;
	_protocolID = frame._protocolID;
	_infoLen = frame._infoLen;
	if (_infoLen > 0)
	{
		_info = new byte[_infoLen + 1];
		memcpy(_info, frame._info, _infoLen);
		_info[_infoLen] = '\0';
	}
}

AX25Frame::~AX25Frame()
{
	delete[] _info;
}

AX25Frame &AX25Frame::operator=(const AX25Frame &frame)
{
	_digipeaterCount = frame._digipeaterCount;
	_addresses = frame._addresses;
	_controlfield = frame._controlfield;
	_protocolID = frame._protocolID;
	_infoLen = frame._infoLen;
	if (_infoLen > 0)
	{
		delete[] _info;
		_info = new byte[_infoLen + 1];
		memcpy(_info, frame._info, _infoLen);
		_info[_infoLen] = '\0';
	}

	return (*this);
}

Ax25Callsign AX25Frame::getDestination()
{
	return _addresses[DESTINATION];
}

Ax25Callsign AX25Frame::getSource()
{
	return _addresses[SOURCE];
}

byte *AX25Frame::getInfoField()
{
	return _info;
}

size_t AX25Frame::getInfoLength()
{
	return _infoLen;
}

byte *AX25Frame::encode(size_t &bufferLen)
{
	byte *outBuffer = nullptr;
	bufferLen = 0;

	// check destination callsign length (6 characters max)
	assert(!_addresses[DESTINATION].getName().empty() && !_addresses[SOURCE].getName().empty());

	// calculate frame length without FCS (destination address, source address, repeater addresses, control, PID, info)
	bufferLen = (2 + _digipeaterCount) * (Ax25Callsign::MAX_CALLSIGN_LEN + 1) + 1 + 1 + _infoLen;
	outBuffer = new byte[bufferLen + 2];
	byte *frameBuffPtr = outBuffer;

	// TODO set callsign
	encodeAddress(_addresses[DESTINATION], frameBuffPtr);
	frameBuffPtr += Ax25Callsign::MAX_CALLSIGN_LEN + 1;
	encodeAddress(_addresses[SOURCE], frameBuffPtr);
	frameBuffPtr += Ax25Callsign::MAX_CALLSIGN_LEN + 1;
	for (int i = 0; i < _digipeaterCount; i++)
	{
		encodeAddress(_addresses[DIGIPEATER1 + i], frameBuffPtr);
		frameBuffPtr += Ax25Callsign::MAX_CALLSIGN_LEN + 1;
	}
	// set HDLC extension end bit
	*(frameBuffPtr - 1) |= SSID_HDLC_EXTENSION_END;
	// set control field
	*(frameBuffPtr++) = _controlfield;
	// set PID field of the frames that have it
	if (_protocolID != 0x00)
	{
		*(frameBuffPtr++) = _protocolID;
	}
	// set info field of the frames that have it
	if (_infoLen > 0)
	{
		memcpy(frameBuffPtr, _info, _infoLen);
	}
	return outBuffer;
}

/**
 * @brief Decode addresses: destination, source, digipeaters
 * Each address consists of a callsign (max. 6 characters) and a 4bit ssid
 * @param buffer pointer to an address
 * @param isLastAddress will be true when the last address has been read.
 * @return AX25Frame::CallSign
 */
Ax25Callsign AX25Frame::decodeAddress(const byte *buffer, bool &isLastAddress)
{
	char name[Ax25Callsign::MAX_CALLSIGN_LEN + 1];
	memset(name, '\0', sizeof(name));
	size_t i = 0;
	while (i < Ax25Callsign::MAX_CALLSIGN_LEN && (buffer[i] != (' ' << 1)))
	{
		name[i] = buffer[i] >> 1;
		i++;
	}
	isLastAddress = (buffer[Ax25Callsign::MAX_CALLSIGN_LEN] & 0x01) != 1 ? false : true;
	byte ssid = (buffer[Ax25Callsign::MAX_CALLSIGN_LEN] >> 1) & 0xF;
	return Ax25Callsign(name, ssid);
}

/**
 * @brief Convert callsign to a byte array so that it can be sent out
 *
 * @param cs 		call sign to be converter
 * @param buffer 	output will be placed here
 */
void AX25Frame::encodeAddress(Ax25Callsign cs, byte *buffer)
{
	// set destination callsign - all address field bytes are shifted by one bit to make room for HDLC address extension bit
	memset(buffer, ' ' << 1, Ax25Callsign::MAX_CALLSIGN_LEN);
	std::string name = cs.getName();
	for (size_t i = 0; i < name.length(); i++)
	{
		*(buffer + i) = name.at(i) << 1;
	}
	buffer[Ax25Callsign::MAX_CALLSIGN_LEN] = SSID_RESERVED_BITS | (cs.getSsid() & 0x0F) << 1;
}