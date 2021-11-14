#include "KissTnc.h"

/**
 * @brief Construct a new KissTnc:: KissTnc object
 * 
 * @param host connection to the host computer (e.g. SerialPort, Bluetooth Serial)
 */
KissTnc::KissTnc(Stream *host) : _host(host)
{
}

KissTnc::~KissTnc()
{
}

void KissTnc::beginPacket()
{
	_txSinglePacketBuffer.clear();
}

size_t KissTnc::write(byte c)
{
	return write(&c, sizeof(byte));
}

size_t KissTnc::write(const byte *buffer, size_t size)
{
	short ctr = 0;
	while (!_txSinglePacketBuffer.isFull() && ctr < size)
	{
		_txSinglePacketBuffer.unshift(buffer[ctr++]);
	}
	return ctr;
}

void KissTnc::flush()
{
	endPacket();
}

/**
 * @brief Construct a packet and send it to the host
 */
void KissTnc::endPacket()
{
	if (!_txSinglePacketBuffer.isEmpty())
	{
		_host->write(FEND);
		_host->write(CMD_DATAFRAME);//Only the "Data frame" code should be sent from the TNC to the host.
		while (_txSinglePacketBuffer.size())
		{
			byte c = _txSinglePacketBuffer.pop();
			switch (c)
			{
			case FEND:
				_host->write(FESC);
				_host->write(TFEND);
				break;
			case FESC:
				_host->write(FESC);
				_host->write(TFESC);
				break;
			default:
				_host->write(c);
			}
		}
		_host->write(FEND);
	}
}

int KissTnc::available()
{
	return _rxSinglePacketBuffer.size();
}

int KissTnc::read()
{
	if (_rxSinglePacketBuffer.isEmpty())
	{
		return -1;
	}
	return _rxSinglePacketBuffer.pop();
}

int KissTnc::peek()
{
	if (_rxSinglePacketBuffer.isEmpty())
	{
		return -1;
	}
	return _rxSinglePacketBuffer.last();
}

void KissTnc::loop()
{
	// Check if we have incoming data to turn into a packet
	while (_host->available())
	{
		byte c = _host->read();
		switch (_rxDataState)
		{
		default:
		case WAIT_FOR_FEND:
			if (c == FEND)
			{
				if (_command == CMD_RETURN)
				{
					if (_onExitKiss != nullptr)
					{
						_onExitKiss();
					}
				}
				_rxDataState = WAIT_FOR_COMMAND;
				_command = CMD_INVALID;
			}
			break;
		case WAIT_FOR_COMMAND:
			//Parse command byte here
			switch (c)
			{
			case FEND:
				//Back-to-back FEND codes should not be interpreted as empty frames. Instead, all but the last FEND code should be discarded.
				break;
			case CMD_RETURN:
				_command = CMD_RETURN;
				_rxDataState = WAIT_FOR_FEND;
				break;
			default:
				if (c > CMD_SETHARDWARE)
				{
					//invalid command, discard the remainder of the frame
					error(c, __FILE__, __LINE__);
					_rxDataState = WAIT_FOR_FEND;
				}
				else
				{
					_command = (COMMANDS)c;
					_rxDataState = WAIT_FOR_DATA;
					_rxSinglePacketBuffer.clear();
				}
				break;
			}
			break;
		case WAIT_FOR_DATA:
			switch (c)
			{
			case FESC:
				_rxDataState = WAIT_FOR_TRANSPOSE;
				break;
			case FEND:
				//Packet finished
				handlePacketReady();
				_rxDataState = WAIT_FOR_COMMAND; //This FEND is also considered as the start of the next frame.
				break;
			default:
				//A normal data byte
				if (!addRxData(c))
				{
					_rxDataState = WAIT_FOR_FEND;
				}
				break;
			}
			break;
		case WAIT_FOR_TRANSPOSE:
			switch (c)
			{
			case TFEND:
				c = FEND;
				break;
			case TFESC:
				c = FESC;
				break;
			default:
				//Only TFEND and TFESC are valid bytes following FESC
				error(c, __FILE__, __LINE__);
				_rxDataState = WAIT_FOR_FEND;
				break;
			}
			_rxDataState = addRxData(c) ? WAIT_FOR_DATA : WAIT_FOR_FEND;
			break;
		}
	}
}

void KissTnc::onExitKiss(void (*callback)(void))
{
	_onExitKiss = callback;
}

void KissTnc::onError(void (*callback)(int, const char *, int))
{
	_onError = callback;
}

void KissTnc::onDataReceived(void (*callback)(int))
{
	_onDataReceived = callback;
}

void KissTnc::onSetHardwareReceived(void (*callback)(int))
{
	_onSetHardwareReceived = callback;
}

void KissTnc::onRadioParameterUpdate(void (*callback)(byte, byte, byte, byte, byte))
{
	_onRadioParameterUpdate = callback;
}

void KissTnc::error(int c, const char *file, int lineNr)
{
	if (_onError != nullptr)
	{
		_onError(c, file, lineNr);
	}
}

bool KissTnc::addRxData(byte c)
{
	//Check data length for commands
	if (_command > CMD_DATAFRAME && _command < CMD_SETHARDWARE && _rxSinglePacketBuffer.size() == 1)
	{
		//these commands only allow for a single data byte
		error(c, __FILE__, __LINE__);
		return false;
	}
	if (!_rxSinglePacketBuffer.unshift(c))
	{
		//RX buffer overflow
		error(c, __FILE__, __LINE__);
		return false;
	}
	return true;
}

void KissTnc::handlePacketReady()
{
	switch (_command)
	{
	case CMD_DATAFRAME:
		if (_onDataReceived != nullptr)
		{
			_onDataReceived(_rxSinglePacketBuffer.size());
		}
		break;
	case CMD_SETHARDWARE:
		if (_onSetHardwareReceived != nullptr)
		{
			_onSetHardwareReceived(_rxSinglePacketBuffer.size());
		}
		break;
	default:
		//Grouping commands which have only one data byte
		switch (_command)
		{
		case CMD_TXDELAY:
			_txDelay = _rxSinglePacketBuffer.pop();
			break;
		case CMD_P:
			_p = _rxSinglePacketBuffer.pop();
			break;
		case CMD_SLOTTIME:
			_slotTime = _rxSinglePacketBuffer.pop();
			break;
		case CMD_TXTAIL:
			_txTail = _rxSinglePacketBuffer.pop();
			break;
		case CMD_FULLDUPLEX:
			_fullDuplex = _rxSinglePacketBuffer.pop();
			break;
		default:
			//Invalid command ()
			error(_command, __FILE__, __LINE__);
			break;
		}
		if (_onRadioParameterUpdate != nullptr)
		{
			_onRadioParameterUpdate(_txDelay, _p, _slotTime, _txTail, _fullDuplex);
		}
	}
}