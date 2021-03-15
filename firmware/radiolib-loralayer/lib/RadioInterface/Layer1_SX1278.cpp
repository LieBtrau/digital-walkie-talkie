
//derived from sudomesh/LoRaLayer2 @ ^1.0.1
#include "Layer1_SX1278.h"

static volatile bool dioFlag;
static volatile bool enableInterrupt = true;

Layer1_SX1278::Layer1_SX1278(SX1278 *lora, int mode, uint8_t sf, float frequency, int power)
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

int Layer1_SX1278::init()
{
	int state = ERR_NONE;
	switch (_mode)
	{
	case 0:
		if (_radio->begin(_frequency, _bandwidth, _spreadingFactor, _codingRate, SX127X_SYNC_WORD, _txPower, _preambleLength, _gain) != ERR_NONE)
		{
			return state;
		}
		break;
	case 1:
		//FDEV = BR/4 to BR*5 and FDEV < 250 - BR/2
		//BW = BR+2*FDEV
		if (_radio->beginFSK(_frequency, 4.8F, 4.8F, 12.5F) != ERR_NONE || //Total bytes : 2400      Total packets : 40      Bitrate : 1920 bps      Average RSSI : -113.09  Average SNR : 0.00
			_radio->setEncoding(RADIOLIB_ENCODING_WHITENING) != ERR_NONE)
		{
			return state;
		}
		break;
	case 2:
		if (_radio->beginFSK(_frequency, 4.8F, 0, 6.3) != ERR_NONE ||
			_radio->setOOK(true) != ERR_NONE ||
			_radio->setEncoding(RADIOLIB_ENCODING_WHITENING) != ERR_NONE)
		{
			return state;
		}
		state = _radio->setDataShapingOOK(1);
		break;
	default:
		return ERR_INVALID_CALLSIGN;
	}
	if (state != ERR_NONE)
	{
		return state;
	}
	_radio->setDio0Action(Layer1_SX1278::setFlag);
	state = _radio->startReceive();
#ifdef LL2_DEBUG
	Serial.printf("Layer1::init(): state = %d\r\n", state);
#endif
	return state;
}

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

int Layer1_SX1278::receive()
{
	if (!dioFlag)
	{
		return ERR_NONE;
	}
	dioFlag = false;
	enableInterrupt = false;
	size_t len = _radio->getPacketLength();
	byte data[len];
	int state = _radio->readData(data, len);

	if (state == ERR_NONE)
	{
		BufferEntry entry;
		memcpy(&entry.data[0], &data[0], len); // copy data to buffer, excluding null terminator
		entry.length = len;
		rxBuffer->write(entry);
		_rssi = _radio->getRSSI();
		_snr = _radio->getSNR();
#ifdef LL2_DEBUG
		Serial.printf("Data length: %d\r\n", len);
		Serial.printf("Layer1::receive(): data = ");
		for (size_t i = 0; i < len; i++)
		{
			Serial.printf("%x ", data[i]);
		}
		Serial.printf("\r\n");
		Serial.printf("Frequency error: %fHz\r\n", _radio->getFrequencyError());
#endif
	}
	else if (state == ERR_RX_TIMEOUT)
	{
		// timeout occurred while waiting for a packet
		Serial.println(F("timeout!"));
	}
	else if (state == ERR_CRC_MISMATCH)
	{
		// packet was received, but is malformed
		Serial.println(F("CRC error!"));
	}
	else
	{
		// some other error occurred
		Serial.print(F("failed, code "));
		Serial.println(state);
	}
	// put module back to listen mode
	_radio->startReceive();

	enableInterrupt = true;
	return state == ERR_NONE ? len : state;
}

int Layer1_SX1278::sendPacket(char *data, size_t len)
{
#ifdef LL2_DEBUG
	Serial.printf("Layer1::sendPacket(): data = ");
	for (size_t i = 0; i < len; i++)
	{
		Serial.printf("%x ", data[i]);
	}
	Serial.printf("\r\n");
#endif
	int state = _radio->transmit((uint8_t *)data, len);
#ifdef LL2_DEBUG
	Serial.printf("Layer1::sendPacket(): state = %d\r\n", state);
#endif
	return state;
}
