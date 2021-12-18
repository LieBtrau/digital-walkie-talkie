// Based on : https://github.com/jgromes/RadioLib/blob/master/src/protocols/AX25/AX25.cpp

#include "Ax25.h"

AX25Frame::AX25Frame(const Ax25Callsign *destCallsign, const Ax25Callsign *srcCallsign, const Ax25Callsign *digipeaterList, size_t digipeaterCount,
					 byte control, byte protocolID, const byte *info, uint16_t infoLen)
{
	if (digipeaterCount > 8)
	{
		return;
	}
	_digipeaterCount = digipeaterCount;
	_addresses = new Ax25Callsign[digipeaterCount + 2];

	// destination callsign/SSID
	_addresses[DESTINATION] = *destCallsign;
	// source callsign/SSID
	_addresses[SOURCE] = *srcCallsign;
	// digipeater list
	for (size_t i = 0; i < digipeaterCount; i++)
	{
		_addresses[DIGIPEATER1 + i] = digipeaterList[i];
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

AX25Frame::AX25Frame(const Ax25Callsign *destCallsign, const Ax25Callsign *srcCallsign, const Ax25Callsign *digipeaterList, size_t digipeaterCount,
					 byte control, byte protocolID, const char *info) : AX25Frame(destCallsign, srcCallsign, digipeaterList, digipeaterCount,
																				  control, protocolID, (const byte *)info, strlen(info))
{
}

AX25Frame::AX25Frame(const byte *ax25data, size_t datalen)
{
	byte *ptrBuf = (byte *)ax25data;
	_addresses = new Ax25Callsign[10]; // not known yet how many addresses really are needed, so allocate the maximum amount.
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
	_addresses = new Ax25Callsign[_digipeaterCount + 2];
	for (size_t i = 0; i < _digipeaterCount + 2; i++)
	{
		_addresses[i] = frame._addresses[i];
	}
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
	delete[] _addresses;
	delete[] _info;
}

AX25Frame &AX25Frame::operator=(const AX25Frame &frame)
{
	_digipeaterCount = frame._digipeaterCount;
	for (size_t i = 0; i < _digipeaterCount + 2; i++)
	{
		_addresses[i] = frame._addresses[i];
	}
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

byte *AX25Frame::encode(size_t &bufferLen)
{
	byte *outBuffer = nullptr;
	bufferLen = 0;

	// check destination callsign length (6 characters max)
	if (_addresses[DESTINATION].getName() == nullptr || _addresses[SOURCE].getName() == nullptr)
	{
		return nullptr;
	}

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
	*(frameBuffPtr - 1) |= RADIOLIB_AX25_SSID_HDLC_EXTENSION_END;
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
	const char *name = cs.getName();
	for (size_t i = 0; i < strlen(name); i++)
	{
		*(buffer + i) = name[i] << 1;
	}
	buffer[Ax25Callsign::MAX_CALLSIGN_LEN] = RADIOLIB_AX25_SSID_RESERVED_BITS | (cs.getSsid() & 0x0F) << 1;
}