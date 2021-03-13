
//derived from sudomesh/LoRaLayer2 @ ^1.0.1
#include "Layer1_SX1278.h"

static volatile bool dioFlag;
static volatile bool enableInterrupt;

Layer1_SX1278::Layer1_SX1278(SX1278 *lora, int mode, uint8_t sf, uint32_t frequency, int power)
	: _radio(lora),
	  _mode(mode),
	  _spreadingFactor(sf),
	  _frequency(frequency),
	  _txPower(power),
	  _spiFrequency(100E3),
	  _bandwidth(125.0),
	  _codingRate(5),
	  _syncWord(SX127X_SYNC_WORD),
	  _currentLimit(100),
	  _preambleLength(8),
	  _gain(0)
{
	txBuffer = new packetBuffer();
	rxBuffer = new packetBuffer();
	dioFlag = false;
	enableInterrupt = true;
};

/* Private functions
*/
// Send packet function
int Layer1_SX1278::sendPacket(char *data, size_t len)
{
#ifdef LL2_DEBUG
	Serial.printf("Layer1::sendPacket(): data = ");
	for (size_t i = 0; i < len; i++)
	{
		Serial.printf("%c", data[i]);
	}
	Serial.printf("\r\n");
#endif
	//data[len] = '\0';
	int state = _radio->startTransmit((uint8_t *)data, len);
#ifdef LL2_DEBUG
	Serial.printf("Layer1::sendPacket(): state = %d\r\n", state);
#endif
	if (state == ERR_NONE)
	{
		_transmitFlag = true;
	}
	return state;
}

// Receive packet callback
void Layer1_SX1278::setFlag(void)
{
	// check if the interrupt is enabled
	if (!enableInterrupt)
	{
		return;
	}
	// we got a packet, set the flag
	dioFlag = true;
}

/*Main public functions
*/
// Initialization
int Layer1_SX1278::init()
{
	int state = ERR_NONE;
	switch (_mode)
	{
	case 0:
		state = _radio->begin(_frequency, _bandwidth, _spreadingFactor, _codingRate, SX127X_SYNC_WORD, _txPower, _preambleLength, _gain);
		break;
	case 1:
		state = _radio->beginFSK(_frequency, 48.0F, 50.0F, _bandwidth, _txPower, _preambleLength);
		break;
	default:
		break;
	}

#ifdef LL2_DEBUG
	Serial.printf("Layer1::init(): state = %d\r\n", state);
#endif
	if (state != ERR_NONE)
	{
		return state;
	}

	_radio->setDio0Action(Layer1_SX1278::setFlag);

	return _radio->startReceive();
}

// Transmit polling function
int Layer1_SX1278::transmit()
{
	BufferEntry entry = txBuffer->read();
	int state = ERR_INVALID_ADDRESS_WIDTH;
	if (entry.length > 0)
	{
#ifdef LL2_DEBUG
		Serial.printf("Layer1::transmit(): entry.length: %d\r\n", entry.length);
#endif
		state = sendPacket(entry.data, entry.length);
	}
	return state == ERR_NONE ? entry.length : state;
}

// Receive polling function
int Layer1_SX1278::receive()
{
	int ret = ERR_NONE;
	int state = 0;
	if (!dioFlag)
	{
		return ERR_NONE;
	}
	if (_transmitFlag)
	{
// interrupt caused by transmit, clear flags and return 0
#ifdef LL2_DEBUG
		Serial.printf("Layer1::receive(): transmit complete\r\n");
#endif
		_transmitFlag = false;
		dioFlag = false;
		_radio->startReceive();
	}
	else
	{
		// interrupt caused by reception
		enableInterrupt = false;
		dioFlag = false;
		size_t len = _radio->getPacketLength();
		if (len > 0)
		{
			byte data[len];
			state = _radio->readData(data, len);
			if (state == ERR_NONE)
			{
				BufferEntry entry;
				memcpy(&entry.data[0], &data[0], len); // copy data to buffer, excluding null terminator
				entry.length = len;
				rxBuffer->write(entry);
#ifdef LL2_DEBUG
				Serial.printf("Data length: %d\r\n", len);
				Serial.printf("Layer1::receive(): data = ");
				for (size_t i = 0; i < len; i++)
				{
					Serial.printf("%c", data[i]);
				}
				Serial.printf("\r\n");
#endif
				ret = len;
			}
		}
		else if (state == ERR_CRC_MISMATCH)
		{
			// packet was received, but is malformed
			ret = -1;
		}
		else
		{
			// some other error occurred
			ret = -2;
		}
		_radio->startReceive();
		enableInterrupt = true;
	}
	return ret;
}
