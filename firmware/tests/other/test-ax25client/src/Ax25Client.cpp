#include "Ax25Client.h"

static Ax25Client *pAx25client = nullptr;
void dataReceivedHandler(int length);

Ax25Client::Ax25Client(KissTnc &tnc, const Ax25Callsign &callsign) : _tnc(&tnc), _sourceAddress(callsign)
{
    _tnc->onDataReceived(dataReceivedHandler);
    pAx25client = this;
}

Ax25Client::~Ax25Client()
{
    delete[] _digipeaterList;
}

void Ax25Client::setDestinationAddress(const Ax25Callsign *callsign)
{
    _destinationAddress = *callsign;
}

void Ax25Client::setDigipeaterAddresses(const Ax25Callsign *list, size_t count)
{
    _digipeaterList = new Ax25Callsign[count];
    for (size_t i = 0; i < count; i++)
    {
        _digipeaterList[i] = list[i];
    }
}

bool Ax25Client::sendFrame(byte control, byte protocolId, const byte *info_field, size_t info_len)
{
    // AX25Frame frame(_destinationAddress, _sourceAddress, _digipeaterList, _digipeaterCount, control, protocolId, info_field, info_len);
    // size_t bufferlen = 0;
    // byte *buffer = frame.encode(bufferlen);
    //  _tnc->beginPacket();
    //  _tnc->write(buffer, bufferlen);
    //  _tnc->endPacket();
    return true;
}

void Ax25Client::setRxFrameCallback(void (*callback)(const Ax25Callsign &destination, const Ax25Callsign &sender, const byte *info_field, size_t info_length))
{
    _rxFrameCallback = callback;
}

void Ax25Client::loop()
{
    _tnc->loop();
}

/**
 * @brief Handle incoming AX.25 frames into the TNC
 * Parse the incoming data buffer and convert it to an AX25Frame-object.  If the callback is not empty, then execute the callback
 * function and hand it over the AX.25 object.
 * @param length number of bytes in the data buffer
 */
void dataReceivedHandler(int length)
{
    byte rxDataBuffer[500];
    long rxDataCounter = 0;

    pAx25client->_tnc->readBytes(rxDataBuffer, length);
    if (pAx25client->_rxFrameCallback != nullptr)
    {
        AX25Frame rxframe((const byte *)rxDataBuffer, length);
        pAx25client->_rxFrameCallback(rxframe.getDestination(), rxframe.getSource(), rxframe.getInfoField(), rxframe.getInfoLength());
    }
    else
    {
        // No callback set, so simply dump received data to serial output
        for (int i = 0; i < length; i++)
        {
            Serial.printf("0x%02x, ", rxDataBuffer[i]);
            if ((++rxDataCounter) % 30 == 0)
            {
                Serial.println();
            }
        }
    }
}