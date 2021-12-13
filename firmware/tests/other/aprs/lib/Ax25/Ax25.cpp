// Based on : https://github.com/jgromes/RadioLib/blob/master/src/protocols/AX25/AX25.cpp

#include "Ax25.h"

AX25Frame::AX25Frame(const char *destCallsign, byte destSSID, const char *srcCallsign, byte srcSSID, byte control)
	: AX25Frame(destCallsign, destSSID, srcCallsign, srcSSID, control, 0, nullptr, 0)
{
}

AX25Frame::AX25Frame(const char *destCallsign, byte destSSID, const char *srcCallsign, byte srcSSID, byte control, byte protocolID, const char *info)
	: AX25Frame(destCallsign, destSSID, srcCallsign, srcSSID, control, protocolID, (byte *)info, strlen(info))
{
}

AX25Frame::AX25Frame(const char *destCallsign, byte destSSID, const char *srcCallsign, byte srcSSID, byte control, byte protocolID, byte *info, uint16_t infoLen)
{
	// destination callsign/SSID
	strcpy(addresses[DESTINATION].name, destCallsign);
	addresses[DESTINATION].ssid = destSSID;

	// source callsign/SSID
	strcpy(addresses[SOURCE].name, srcCallsign);
	addresses[SOURCE].ssid = srcSSID;

	// control field
	this->controlfield = control;

	// PID field
	this->protocolID = protocolID;

	// info field
	this->infoLen = infoLen;
	if (infoLen > 0)
	{
		this->info = new byte[infoLen];
		memcpy(this->info, info, infoLen);
	}
}

AX25Frame::AX25Frame(const byte *buffer, size_t bufferLen)
{
	byte *ptrBuf = (byte *)buffer;
	CallSign *ptrCallsign = addresses;
	bool isLastAddress;
	// Decode callsigns : destination, source, digipeaters
	do
	{
		*(ptrCallsign++) = decodeAddress(ptrBuf, isLastAddress);
		ptrBuf += 7;
	} while ((ptrBuf < buffer + 70) && (!isLastAddress));
	digipeaterCount = ptrCallsign - addresses - 2;
	// Decode control field.  For APRS, it's always 0x03.
	controlfield = *(ptrBuf++);
	// Decode protocol ID.  For APRS, it's always 0xF0 (no layer 3 protocol)
	protocolID = *(ptrBuf++);
	// Get the data from the information field
	infoLen = buffer + bufferLen - ptrBuf;
	info = new byte[infoLen + 1];
	memcpy(info, ptrBuf, infoLen);
	info[infoLen] = '\0';
}

AX25Frame::AX25Frame(const AX25Frame &frame)
{
	*this = frame;
}

AX25Frame::~AX25Frame()
{
	// deallocate info field
	if (infoLen > 0)
	{
		delete[] this->info;
	}
}

// AX25Frame &AX25Frame::operator=(const AX25Frame &frame)
// {
// 	// destination callsign/SSID
// 	strcpy(this->destCallsign, frame.destCallsign);
// 	this->destSSID = frame.destSSID;

// 	// source callsign/SSID
// 	strcpy(this->srcCallsign, frame.srcCallsign);
// 	this->srcSSID = frame.srcSSID;

// 	// control field
// 	this->control = frame.control;

// 	// PID field
// 	this->protocolID = frame.protocolID;

// 	// info field
// 	this->infoLen = frame.infoLen;
// 	memcpy(this->info, frame.info, this->infoLen);

// 	return (*this);
// }

byte * AX25Frame::encode(size_t &bufferLen)
{
	byte* outBuffer = nullptr;
	bufferLen = 0;

	// check destination callsign length (6 characters max)
	if (strlen(addresses[DESTINATION].name) > RADIOLIB_AX25_MAX_CALLSIGN_LEN)
	{
		return nullptr;
	}

	// calculate frame length without FCS (destination address, source address, repeater addresses, control, PID, info)
	bufferLen = (2 + digipeaterCount) * (RADIOLIB_AX25_MAX_CALLSIGN_LEN + 1) + 1 + 1 + infoLen;
	outBuffer = new byte[bufferLen + 2];
	byte *frameBuffPtr = outBuffer;

	// TODO set callsign
	encodeAddress(addresses[DESTINATION], frameBuffPtr);
	frameBuffPtr += 7;
	encodeAddress(addresses[SOURCE], frameBuffPtr);
	frameBuffPtr += 7;
	for (int i = 0; i < digipeaterCount; i++)
	{
		encodeAddress(addresses[DIGIPEATER1 + i], frameBuffPtr);
		frameBuffPtr += 7;
	}
	// set HDLC extension end bit
	*(frameBuffPtr - 1) |= RADIOLIB_AX25_SSID_HDLC_EXTENSION_END;
	// set control field
	*(frameBuffPtr++) = controlfield;
	// set PID field of the frames that have it
	if (protocolID != 0x00)
	{
		*(frameBuffPtr++) = protocolID;
	}
	// set info field of the frames that have it
	if (infoLen > 0)
	{
		memcpy(frameBuffPtr, info, infoLen);
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
AX25Frame::CallSign AX25Frame::decodeAddress(const byte *buffer, bool &isLastAddress)
{
	CallSign callsign;
	for (size_t i = 0; i < 6; i++)
	{
		callsign.name[i] = (buffer[i] != 0x40 ? buffer[i] : '\0') >> 1;
	}
	isLastAddress = (buffer[6] & 0x01) != 1 ? false : true;
	callsign.ssid = (buffer[6] >> 1) & 0xF;
	return callsign;
}

/**
 * @brief Convert callsign to a byte array so that it can be sent out
 * 
 * @param cs 		call sign to be converter
 * @param buffer 	output will be placed here
 */
void AX25Frame::encodeAddress(CallSign cs, byte *buffer)
{
	// set destination callsign - all address field bytes are shifted by one bit to make room for HDLC address extension bit
	memset(buffer, ' ' << 1, RADIOLIB_AX25_MAX_CALLSIGN_LEN);
	for (size_t i = 0; i < strlen(cs.name); i++)
	{
		*(buffer + i) = cs.name[i] << 1;
	}
	buffer[6] = RADIOLIB_AX25_SSID_RESERVED_BITS | (cs.ssid & 0x0F) << 1;
}