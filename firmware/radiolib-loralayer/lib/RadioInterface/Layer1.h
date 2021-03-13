#pragma once

#include "packetBuffer.h"

class Layer1
{
public:
    virtual int init() = 0;
    virtual int transmit() = 0;
    virtual int receive() = 0;
    virtual float getRSSI() { return _rssi; };
    virtual float getSNR() { return _snr; };

    // Fifo buffers
    packetBuffer *txBuffer;
    packetBuffer *rxBuffer;

protected:
    float _snr = 0;
    float _rssi = 0;
};
