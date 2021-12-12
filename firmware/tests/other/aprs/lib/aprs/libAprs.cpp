#include "libAprs.h"

libAprs::libAprs(byte dti) : dataTypeId((DTI)dti)
{
}

libAprs::~libAprs()
{
	delete[] dataExtension;
	delete[] comment;
}

libAprs *libAprs::fromAprs(byte *buffer, size_t len)
{
	libAprs *aprsPacket = nullptr;
	switch (buffer[0])
	{
	case MESSAGE:
		// Message packet
		aprsPacket = new AprsMessage(buffer, len);
		break;
	case POS_NO_TIME_NO_MSG:
	case POS_NO_TIME_WITH_MSG:
	case POS_WITH_TIME_WITH_MSG:
	case POS_WITH_TIME_NO_MSG:
		aprsPacket = new AprsPositionReport(buffer, len);
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
bool libAprs::hasAprsExtension(const byte *buffer)
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

libAprs::PACKET_TYPE libAprs::getPacketType()
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

void libAprs::setComment(const byte *buffer, byte len)
{
	commentLen = len;
	comment = new byte[len + 1];
	memcpy(comment, buffer, len);
	comment[len] = '\0';
}

AprsPositionReport::~AprsPositionReport()
{
	delete[] latitude;
	delete[] longitude;
}

AprsPositionReport::AprsPositionReport(const byte *buffer, size_t len) : libAprs(buffer[0])
{
	switch (dataTypeId)
	{
	case POS_NO_TIME_NO_MSG:
	case POS_NO_TIME_WITH_MSG:
		// Parse 20 byte header
		// latitude
		latitude = new char[9];
		memset(latitude, '\0', 9);
		memcpy(latitude, buffer + 1, 8);
		// symbol table ID
		symbolTableId = buffer[9];
		// longitude
		longitude = new char[10];
		memset(longitude, '\0', 10);
		memcpy(longitude, buffer + 10, 9);
		// symbol code
		symbolCode = buffer[19];
		if (hasAprsExtension(buffer + 20))
		{
			// Parse extension data
			//...
			// Copy comment data
			commentLen = len - 27;
			memcpy(comment, buffer + 27, commentLen);
		}
		else
		{
			setComment(buffer + 20, len - 20);
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

char *AprsPositionReport::toAprs()
{
	return nullptr;
}

AprsMessage::AprsMessage(const byte *buffer, size_t len) : libAprs(buffer[0])
{
	if (buffer[10] != ':')
	{
		// invalid message
		return;
	}
	// Get Addressee
	addressee = new char[10];
	memset(addressee, '\0', 10);
	memcpy(addressee, buffer + 1, 9);
	char *endOfAddressee = strchr(addressee, ' ');
	if (endOfAddressee != nullptr)
	{
		*endOfAddressee = '\0';
	}
	// Get Message Text
	messageText = new char[len - 11 + 1];
	memcpy(messageText, buffer + 11, len - 11);
	messageText[len - 11] = '\0';

	// Check if the message is an acknowledgement or a reject
	// Why doesn't the APRS standard include a '{' in the acknowledgement?  It would have made parsing so much easier.
	if (((strstr(messageText, "ack") == messageText) || (strstr(messageText, "rej") == messageText)) && strspn(messageText + 3, "0123456789") == strlen(messageText + 3))
	{
		// Yes, it's an acknowledgement
		messageNo = atoi(messageText + 3);
		messageText[3] = '\0';
	}
	else
	{
		// Get Message No (if present)
		char *messageIdPtr = strchr(messageText, '{');
		if (messageIdPtr != nullptr)
		{
			// Break away the messageID from the message Text
			*messageIdPtr = '\0';
			messageNo = atoi(messageIdPtr + 1);
		}
	}
}

AprsMessage::~AprsMessage()
{
	delete[] addressee;
	delete[] messageText;
}

const char *AprsMessage::getAddressee()
{
	return addressee;
}

const char *AprsMessage::getMessage()
{
	return messageText;
}

int AprsMessage::getMessageId()
{
	return messageNo;
}

bool AprsMessage::setMessageText(const char *text, int msgNr)
{
	size_t bufferlen = strlen(text);
	// Check maximum length
	if (strlen(text) > 67)
	{
		return false;
	}
	// Scan for forbidden characters
	if (strpbrk(text, "|~{") != nullptr)
	{
		return false;
	}
	delete[] messageText;
	messageText = new char[bufferlen + 1];
	strcpy(messageText, text);
	messageNo = msgNr;
	return true;
}

/**
 * @brief Convert the object to an APRS-string
 * 
 * @return char* string containing the APRS-data.  The user is responsible for freeing up the memory afterwards.
 */
char *AprsMessage::toAprs()
{
	size_t messageLen = strlen(messageText);
	char *outputBuffer = new char[11 + messageLen + 1 + 5];
	// Addressee
	outputBuffer[0] = ':';
	memset(outputBuffer + 1, ' ', 9);
	memcpy(outputBuffer + 1, addressee, strlen(addressee));
	outputBuffer[10] = ':';
	// MessageText
	strcpy(outputBuffer + 11, messageText);
	// Message No
	if (messageNo == 0)
	{
		return outputBuffer;
	}
	outputBuffer[11 + messageLen] = '{';
	sprintf((char *)outputBuffer + 11 + messageLen + 1, "%d", messageNo);
	return outputBuffer;
}