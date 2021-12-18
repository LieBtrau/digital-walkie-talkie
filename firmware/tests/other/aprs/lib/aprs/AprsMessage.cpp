#include "AprsMessage.h"

AprsMessage::AprsMessage(const byte *ax25_information_field, size_t info_len) : AprsPacket(ax25_information_field[0])
{
	if (ax25_information_field[10] != ':')
	{
		// invalid message
		return;
	}
	// Get Addressee
	addressee = new char[10];
	memset(addressee, '\0', 10);
	memcpy(addressee, ax25_information_field + 1, 9);
	char *endOfAddressee = strchr(addressee, ' ');
	if (endOfAddressee != nullptr)
	{
		*endOfAddressee = '\0';
	}
	// Get Message Text
	messageText = new char[info_len - 11 + 1];
	memcpy(messageText, ax25_information_field + 11, info_len - 11);
	messageText[info_len - 11] = '\0';

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
char *AprsMessage::encode()
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