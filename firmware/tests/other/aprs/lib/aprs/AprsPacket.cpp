#include "AprsMessage.h"
#include "AprsPositionReport.h"

AprsPacket::AprsPacket(byte dti) : _dataTypeId((DTI)dti)
{
}

AprsPacket::~AprsPacket()
{
	delete[] dataExtension;
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
 * See APRS Protocol Version 1.0.1 : Chapter 7: APRS Data Extensions
 * @param buffer pointer to the start of the expected APRS-extension.
 * @return true APRS extension presetn
 * @return false no APRS extension
 */
bool AprsPacket::hasAprsExtension(const std::string& buffer)
{
	if (buffer.at(3) == '/')
	{
		return true;
	}
	if (buffer.find("PHG") == 0 || buffer.find("RNG") == 0 || buffer.find("DFS") == 0)
	{
		return true;
	}
	// Area Object Descriptor is hard to parse.  Anyone using this?
	return false;
}

AprsPacket::PACKET_TYPE AprsPacket::getPacketType()
{
	switch (_dataTypeId)
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

bool AprsPacket::setComment(const std::string& comment)
{
	_comment = comment;
	//The comment may contain any printable ASCII characters (except | and ~, which are reserved for TNC channel switching)
	return comment.find_first_of("|~") == std::string::npos ? true : false;
}

std::string AprsPacket::getComment()
{
	return _comment;
}