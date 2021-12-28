#include "AprsMessage.h"

AprsMessage::AprsMessage(const byte *ax25_information_field, size_t info_len) : AprsPacket(ax25_information_field[0])
{
	assert(ax25_information_field[10] == ':');
	// Get Addressee
	for (int i = 1; i < 10; i++)
	{
		if(ax25_information_field[i]!=' ')
		{
		_addressee += ax25_information_field[i];
		}
		else
		{
			break;
		}
	}
	// Get Message Text
	_messageText+=(char*)(ax25_information_field+11);

	// Check if the message is an acknowledgement or a reject
	// Why doesn't the APRS standard include a '{' in the acknowledgement?  It would have made parsing so much easier.
	if (((_messageText.find("ack") == 0) || (_messageText.find("rej") == 0)) && _messageText.find_first_not_of("0123456789", 3) == std::string::npos)
	{
		// Yes, it's an acknowledgement or a reject
		_messageNo = atoi(_messageText.substr(3).c_str());
		_messageText.erase(3);
	}
	else
	{
		// Get Message No (if present)
		std::size_t bracketPos = _messageText.find('{');
		if (bracketPos != std::string::npos)
		{
			// Break away the messageID from the message Text
			_messageNo = atoi(_messageText.substr(bracketPos + 1).c_str());
			_messageText.erase(bracketPos);
		}
	}
}

AprsMessage::AprsMessage(const char *text, int msgNr) : AprsPacket(AprsPacket::MESSAGE)
{
	setMessageText(text, msgNr);
}

AprsMessage::~AprsMessage()
{
}

const std::string AprsMessage::getAddressee() const
{
	return _addressee;
}

const char *AprsMessage::getMessage() const
{
	return _messageText.c_str();
}

int AprsMessage::getMessageId() const
{
	return _messageNo;
}

bool AprsMessage::setMessageText(const std::string text, int msgNr)
{
	// Check maximum length
	if (text.length() > 67)
	{
		return false;
	}
	// Scan for forbidden characters
	if (text.find_first_of("|~{") != std::string::npos)
	{
		return false;
	}
	_messageText = text;
	_messageNo = msgNr;
	return true;
}

bool AprsMessage::setAddressee(const std::string addressee)
{
	if (addressee.length() > 6)
	{
		return false;
	}
	_addressee = addressee;
	return true;
}

/**
 * @brief Convert the object to an APRS-string
 *
 * @return char* string containing the APRS-data.
 */
const char *AprsMessage::encode() const
{
	assert(!_addressee.empty());
	assert(getMessageType() != MSG_NOT_DEFINED);
	std::string outputBuffer;
	// Addressee
	outputBuffer += ':';
	outputBuffer.append(_addressee);
	while (outputBuffer.length() < 10)
		outputBuffer += ' ';
	outputBuffer += ':';
	// MessageText
	outputBuffer.append(_messageText);
	// Message No
	if (_messageNo == 0)
	{
		return outputBuffer.c_str();
	}
	if (getMessageType() == MSG_PLAIN)
	{
		outputBuffer += '{';
	}
	char buffer[6];
	itoa(_messageNo, buffer, 10);
	outputBuffer += buffer;
	return outputBuffer.c_str();
}

bool AprsMessage::isAckRequired() const
{
	return getMessageType() == MSG_PLAIN && _messageNo > 0;
}

AprsMessage::MESSAGE_TYPE AprsMessage::getMessageType() const
{
	if (_messageText.empty() || _messageText.length() == 0)
	{
		return MSG_NOT_DEFINED;
	}
	if (_messageText.compare("ack") == 0 && _messageNo > 0)
	{
		return MSG_ACK;
	}
	if (_messageText.compare("rej") == 0 && _messageNo > 0)
	{
		return MSG_REJECT;
	}
	return MSG_PLAIN;
}