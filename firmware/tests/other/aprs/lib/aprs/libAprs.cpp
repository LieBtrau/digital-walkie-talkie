#include "AprsMessage.h"
#include "AprsPositionReport.h"

AprsPacket::AprsPacket(byte dti) : dataTypeId((DTI)dti)
{
}

AprsPacket::~AprsPacket()
{
	delete[] dataExtension;
	delete[] comment;
}

/**
 * @brief Decode a byte array that contains APRS-data
 * 
 * @param ax25_information_field pointer to a byte array with APRS-data
 * @param info_len the number of bytes in the APRS-packet inside the buffer
 * @return libAprs* pointer to a derived class of libAprs
 */
AprsPacket *AprsPacket::decode(const byte *ax25_information_field, size_t info_len)
{
	AprsPacket *aprsPacket = nullptr;
	switch (ax25_information_field[0])
	{
	case MESSAGE:
		// Message packet
		aprsPacket = new AprsMessage(ax25_information_field, info_len);
		break;
	case POS_NO_TIME_NO_MSG:
	case POS_NO_TIME_WITH_MSG:
	case POS_WITH_TIME_WITH_MSG:
	case POS_WITH_TIME_NO_MSG:
		aprsPacket = new AprsPositionReport(ax25_information_field, info_len);
		break;
	default:
		// Unsupported packet
		break;
	}

	return aprsPacket;
}

/**
 * @brief Check if the APRS packet contains APRS extension data or not.
 * APRS really sucks in this respect.  There's no absolute reliable way to determine whether the APRS extension is present.
 * The patterns checked here for, may also be present in the comment field.
 * @param buffer pointer to the start of the expected APRS-extension.
 * @return true APRS extension presetn
 * @return false no APRS extension
 */
bool AprsPacket::hasAprsExtension(const byte *buffer)
{
	if (buffer[3] == '/')
	{
		return true;
	}
	if (strncmp("PHG", (const char *)buffer, strlen("PHG") == 0))
	{
		return true;
	}
	if (strncmp("RNG", (const char *)buffer, strlen("RNG") == 0))
	{
		return true;
	}
	if (strncmp("DFS", (const char *)buffer, strlen("DFS") == 0))
	{
		return true;
	}
	// Area Object Descriptor is hard to parse.  Anyone using this?
	return false;
}

AprsPacket::PACKET_TYPE AprsPacket::getPacketType()
{
	switch (dataTypeId)
	{
	case MESSAGE:
		return PKT_TEXT;
	case POS_NO_TIME_NO_MSG:
	case POS_NO_TIME_WITH_MSG:
	case POS_WITH_TIME_WITH_MSG:
	case POS_WITH_TIME_NO_MSG:
		return PKT_LOCATION;
	default:
		return PKT_UNSUPPORTED;
	}
}

bool AprsPacket::setComment(const byte *buffer, byte len)
{
	commentLen = len;
	comment = new byte[len + 1];
	memcpy(comment, buffer, len);
	comment[len] = '\0';
	//The comment may contain any printable ASCII characters (except | and ~, which are reserved for TNC channel switching).
	return strpbrk((const char*)comment, "|~") == nullptr ? false : true;
}