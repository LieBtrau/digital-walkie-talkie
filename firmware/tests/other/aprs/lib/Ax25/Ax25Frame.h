// Based on : https://github.com/jgromes/RadioLib/blob/master/src/protocols/AX25/AX25.h

#pragma once

#include "array"
#include "Arduino.h"
#include "Ax25Callsign.h"

/*!
  \class AX25Frame

  \brief Abstraction of AX.25 frame format.
*/
class AX25Frame
{
public:
	/*!
		\brief Default constructor.
		\param destCallsign Callsign of the destination station.
		\param destSSID SSID of the destination station.
		\param srcCallsign Callsign of the source station.
		\param srcSSID SSID of the source station.
		\param control The control field.
		\param protocolID The protocol identifier (PID) field. Set to zero if the frame doesn't have this field.
		\param info Information field, in the form of arbitrary binary buffer.
		\param infoLen Number of bytes in the information field.
	  */
	AX25Frame(const Ax25Callsign &destCallsign, const Ax25Callsign &srcCallsign, const std::array<Ax25Callsign, 8> digipeaterList, byte control, byte protocolID, const byte *info, uint16_t infoLen);

	AX25Frame(const Ax25Callsign &destCallsign, const Ax25Callsign &srcCallsign, const std::array<Ax25Callsign, 8> digipeaterList, byte control, byte protocolID, std::string info);

	AX25Frame(const byte *outBuffer, size_t bufferLen);
	/*!
		\brief Copy constructor.
		\param frame AX25Frame instance to copy.
	  */
	AX25Frame(const AX25Frame &frame);

	/*!
		\brief Default destructor.
	  */
	~AX25Frame();

	/*!
		\brief Overload for assignment operator.
		\param frame rvalue AX25Frame.
	  */
	AX25Frame &operator=(const AX25Frame &frame);

	/**
	 * @brief Get the raw AX25-frame
	 * @param bufferLen number of bytes in the databuffer
	 * @return byte* will point to a buffer containing the AX25 bytes upon return.  You'll need to release this buffer yourself.
	 */
	byte *encode(size_t &bufferLen);

	Ax25Callsign getDestination();
	Ax25Callsign getSource();
	byte *getInfoField();
	size_t getInfoLength();

private:
	const byte SSID_RESERVED_BITS = 0b01100000;		 //  6     5     reserved bits in SSID
	const byte SSID_HDLC_EXTENSION_END = 0b00000001; //  0     0     address field end
	typedef enum
	{
		DESTINATION,
		SOURCE,
		DIGIPEATER1,
		DIGIPEATER2,
		DIGIPEATER3,
		DIGIPEATER4,
		DIGIPEATER5,
		DIGIPEATER6,
		DIGIPEATER7,
		DIGIPEATER8
	} Address;
	byte _digipeaterCount = 0;
	std::array<Ax25Callsign, 10> _addresses;
	byte _controlfield = 0; //!< The control field.
	byte _protocolID = 0;	//!< The protocol identifier (PID) field.
	word _infoLen = 0;		//!< Number of bytes in the information field.
	byte *_info = nullptr;	//!< The info field.
	Ax25Callsign decodeAddress(const byte *buffer, bool &isLastAddress);
	void encodeAddress(Ax25Callsign cs, byte *buffer);
};
