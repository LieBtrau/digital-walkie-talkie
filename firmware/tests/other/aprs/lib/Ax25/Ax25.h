// Based on : https://github.com/jgromes/RadioLib/blob/master/src/protocols/AX25/AX25.h

#pragma once

#include "Arduino.h"
#include "Ax25Callsign.h"

// // macros to access bits in byte array, from http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
// #define SET_BIT_IN_ARRAY(A, k) (A[(k / 8)] |= (1 << (k % 8)))
// #define CLEAR_BIT_IN_ARRAY(A, k) (A[(k / 8)] &= ~(1 << (k % 8)))
// #define TEST_BIT_IN_ARRAY(A, k) (A[(k / 8)] & (1 << (k % 8)))
// #define GET_BIT_IN_ARRAY(A, k) ((A[(k / 8)] & (1 << (k % 8))) ? 1 : 0)

// // CRC-CCITT calculation macros
// #define XOR(A, B) (((A) || (B)) && !((A) && (B)))
// #define CRC_CCITT_POLY 0x1021		   //  generator polynomial
// #define CRC_CCITT_POLY_REVERSED 0x8408 //  CRC_CCITT_POLY in reversed bit order
// #define CRC_CCITT_INIT 0xFFFF		   //  initial value

// // maximum callsign length in bytes

// // flag field                                                         MSB   LSB   DESCRIPTION
// #define RADIOLIB_AX25_FLAG 0b01111110 //  7     0     AX.25 frame start/end flag

// // address field
// #define RADIOLIB_AX25_SSID_COMMAND_DEST 0b10000000			  //  7     7     frame type: command (set in destination SSID)
// #define RADIOLIB_AX25_SSID_COMMAND_SOURCE 0b00000000		  //  7     7                 command (set in source SSID)
// #define RADIOLIB_AX25_SSID_RESPONSE_DEST 0b00000000			  //  7     7                 response (set in destination SSID)
// #define RADIOLIB_AX25_SSID_RESPONSE_SOURCE 0b10000000		  //  7     7                 response (set in source SSID)
// #define RADIOLIB_AX25_SSID_HAS_NOT_BEEN_REPEATED 0b00000000	  //  7     7                 not repeated yet (set in repeater SSID)
// #define RADIOLIB_AX25_SSID_HAS_BEEN_REPEATED 0b10000000		  //  7     7                 repeated (set in repeater SSID)
#define RADIOLIB_AX25_SSID_RESERVED_BITS 0b01100000			  //  6     5     reserved bits in SSID
// #define RADIOLIB_AX25_SSID_HDLC_EXTENSION_CONTINUE 0b00000000 //  0     0     HDLC extension bit: next octet contains more address information
#define RADIOLIB_AX25_SSID_HDLC_EXTENSION_END 0b00000001	  //  0     0                         address field end

// // control field
// #define RADIOLIB_AX25_CONTROL_U_SET_ASYNC_BAL_MODE 0b01101100	  //  7     2     U frame type: set asynchronous balanced mode (connect request)
// #define RADIOLIB_AX25_CONTROL_U_SET_ASYNC_BAL_MODE_EXT 0b00101100 //  7     2                   set asynchronous balanced mode extended (connect request with module 128)
// #define RADIOLIB_AX25_CONTROL_U_DISCONNECT 0b01000000			  //  7     2                   disconnect request
// #define RADIOLIB_AX25_CONTROL_U_DISCONNECT_MODE 0b00001100		  //  7     2                   disconnect mode (system busy or disconnected)
// #define RADIOLIB_AX25_CONTROL_U_UNNUMBERED_ACK 0b01100000		  //  7     2                   unnumbered acknowledge
// #define RADIOLIB_AX25_CONTROL_U_FRAME_REJECT 0b10000100			  //  7     2                   frame reject
// #define RADIOLIB_AX25_CONTROL_U_UNNUMBERED_INFORMATION 0b00000000 //  7     2                   unnumbered information
// #define RADIOLIB_AX25_CONTROL_U_EXHANGE_IDENTIFICATION 0b10101100 //  7     2                   exchange ID
// #define RADIOLIB_AX25_CONTROL_U_TEST 0b11100000					  //  7     2                   test
// #define RADIOLIB_AX25_CONTROL_POLL_FINAL_ENABLED 0b00010000		  //  4     4     control field poll/final bit: enabled
// #define RADIOLIB_AX25_CONTROL_POLL_FINAL_DISABLED 0b00000000	  //  4     4                                   disabled
// #define RADIOLIB_AX25_CONTROL_S_RECEIVE_READY 0b00000000		  //  3     2     S frame type: receive ready (system ready to receive)
// #define RADIOLIB_AX25_CONTROL_S_RECEIVE_NOT_READY 0b00000100	  //  3     2                   receive not ready (TNC buffer full)
// #define RADIOLIB_AX25_CONTROL_S_REJECT 0b00001000				  //  3     2                   reject (out of sequence or duplicate)
// #define RADIOLIB_AX25_CONTROL_S_SELECTIVE_REJECT 0b00001100		  //  3     2                   selective reject (single frame repeat request)
// #define RADIOLIB_AX25_CONTROL_INFORMATION_FRAME 0b00000000		  //  0     0     frame type: information (I frame)
// #define RADIOLIB_AX25_CONTROL_SUPERVISORY_FRAME 0b00000001		  //  1     0                 supervisory (S frame)
// #define RADIOLIB_AX25_CONTROL_UNNUMBERED_FRAME 0b00000011		  //  1     0                 unnumbered (U frame)

// // protocol identifier field
// #define RADIOLIB_AX25_PID_ISO_8208 0x01
// #define RADIOLIB_AX25_PID_TCP_IP_COMPRESSED 0x06
// #define RADIOLIB_AX25_PID_TCP_IP_UNCOMPRESSED 0x07
// #define RADIOLIB_AX25_PID_SEGMENTATION_FRAGMENT 0x08
// #define RADIOLIB_AX25_PID_TEXNET_DATAGRAM_PROTOCOL 0xC3
// #define RADIOLIB_AX25_PID_LINK_QUALITY_PROTOCOL 0xC4
// #define RADIOLIB_AX25_PID_APPLETALK 0xCA
// #define RADIOLIB_AX25_PID_APPLETALK_ARP 0xCB
// #define RADIOLIB_AX25_PID_ARPA_INTERNET_PROTOCOL 0xCC
// #define RADIOLIB_AX25_PID_ARPA_ADDRESS_RESOLUTION 0xCD
// #define RADIOLIB_AX25_PID_FLEXNET 0xCE
// #define RADIOLIB_AX25_PID_NET_ROM 0xCF
// #define RADIOLIB_AX25_PID_NO_LAYER_3 0xF0
// #define RADIOLIB_AX25_PID_ESCAPE_CHARACTER 0xFF

/*!
  \class AX25Frame

  \brief Abstraction of AX.25 frame format.
*/
class AX25Frame
{
public:
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
	Ax25Callsign* _addresses=nullptr;
	byte _controlfield = 0; //!< The control field.
	byte _protocolID = 0;   //!< The protocol identifier (PID) field.
	word _infoLen = 0;	   //!< Number of bytes in the information field.
	byte *_info = nullptr;  //!< The info field.

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
	AX25Frame(const Ax25Callsign *destCallsign, const Ax25Callsign *srcCallsign, const Ax25Callsign *digipeaterList, size_t digipeaterCount,
			  byte control, byte protocolID, const byte *info, uint16_t infoLen);

	AX25Frame(const Ax25Callsign *destCallsign, const Ax25Callsign *srcCallsign, const Ax25Callsign *digipeaterList, size_t digipeaterCount,
			  byte control, byte protocolID, const char *info);

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

private:
	Ax25Callsign decodeAddress(const byte *buffer, bool &isLastAddress);
	void encodeAddress(Ax25Callsign cs, byte *buffer);
};
