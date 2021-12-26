#include "AprsPositionReport.h"

AprsPositionReport::~AprsPositionReport()
{
	delete[] latitude;
	delete[] longitude;
}

AprsPositionReport::AprsPositionReport(const byte *ax25_information_field, size_t info_len) : AprsPacket(ax25_information_field[0])
{
	switch (dataTypeId)
	{
	case POS_NO_TIME_NO_MSG:
	case POS_NO_TIME_WITH_MSG:
		// Parse 20 byte header
		// latitude
		latitude = new char[9];
		memset(latitude, '\0', 9);
		memcpy(latitude, ax25_information_field + 1, 8);
		// symbol table ID
		symbolTableId = ax25_information_field[9];
		// longitude
		longitude = new char[10];
		memset(longitude, '\0', 10);
		memcpy(longitude, ax25_information_field + 10, 9);
		// symbol code
		symbolCode = ax25_information_field[19];
		if (hasAprsExtension(ax25_information_field + 20))
		{
			// Parse extension data
			//...
			// Copy comment data
			commentLen = info_len - 27;
			memcpy(comment, ax25_information_field + 27, commentLen);
		}
		else
		{
			setComment(ax25_information_field + 20, info_len - 20);
		}
		break;
	case POS_WITH_TIME_WITH_MSG:
	case POS_WITH_TIME_NO_MSG:
		// Parse 27 byte header
		break;
	default:
		break;
	}
}

const char *AprsPositionReport::getLatitude()
{
	return latitude;
}

const char *AprsPositionReport::getLongitude()
{
	return longitude;
}

byte AprsPositionReport::getSymbolTableId()
{
	return symbolTableId;
}

byte AprsPositionReport::getSymbolCode()
{
	return symbolCode;
}

const char *AprsPositionReport::encode() const
{
	return nullptr;
}