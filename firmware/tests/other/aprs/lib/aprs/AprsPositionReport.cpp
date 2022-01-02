#include "AprsPositionReport.h"

AprsPositionReport::~AprsPositionReport()
{
}

AprsPositionReport::AprsPositionReport(const byte *ax25_information_field, size_t info_len) : AprsPacket(ax25_information_field[0])
{
	assert(info_len < MAX_INFOFIELD_LEN);

	std::string information_field(reinterpret_cast<const char *>(ax25_information_field), info_len);

	switch (_dataTypeId)
	{
	case POS_NO_TIME_NO_MSG:
	case POS_NO_TIME_WITH_MSG:
		// Parse 20 byte header
		// latitude, with conversion from decimal minutes to decimal degrees
		_location.setLatitude(strtod(information_field.substr(1, 7).c_str(), nullptr) / (information_field.at(8) == 'S' ? -100 : 100));
		// symbol table ID
		_symbol.setTableId(information_field.at(9));
		// longitude, with conversion from decimal minutes to decimal degrees
		_location.setLongitude(strtod(information_field.substr(10, 8).c_str(), nullptr) / (information_field.at(18) == 'W'? -100 : 100));
		// symbol code
		_symbol.setSymbol(information_field.at(19));
		if (hasAprsExtension(information_field.substr(20)))
		{
			// Parse extension data
			//...
			// Copy comment data
			_comment = information_field.substr(27);
		}
		else
		{
			_comment = information_field.substr(20);
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

AprsPositionReport::AprsPositionReport(const AprsLocation &location, const std::string &comment) : AprsPacket(POS_NO_TIME_WITH_MSG)
{
	_location = location;
	assert(setComment(comment));
}

AprsLocation AprsPositionReport::getPosition() const
{
	return _location;
}

AprsSymbol AprsPositionReport::getSymbol() const
{
	return _symbol;
}

void AprsPositionReport::setLocation(const AprsLocation &location)
{
	_location = location;
}

void AprsPositionReport::setSymbol(const AprsSymbol& symbol)
{
	_symbol = symbol;
}

const std::string AprsPositionReport::encode() const
{
	std::string posReport;
	switch (_dataTypeId)
	{
	case POS_NO_TIME_WITH_MSG:
		posReport = POS_NO_TIME_WITH_MSG;
		posReport += _location.encodeLatitude();
		posReport += _symbol.encodeTableId();
		posReport += _location.encodeLongitude();
		posReport += _symbol.encodeSymbol();
		posReport += _comment;
		break;
	default:
		return std::string("");
	}
	return posReport;
}