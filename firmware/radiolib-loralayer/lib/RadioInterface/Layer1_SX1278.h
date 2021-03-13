//derived from sudomesh/LoRaLayer2 @ ^1.0.1

#pragma once

#include <RadioLib.h>
#include "Layer1.h"

class Layer1_SX1278 : public Layer1
{

public:
    Layer1_SX1278(SX1278 *lora, int mode, uint8_t sf = 9, uint32_t frequency = 434, int power = 17);

    // Main public functions
    int init();
    int transmit();
    int receive();

private:
    // Main private functions
    static void setFlag(void);
    int sendPacket(char *data, size_t len);

    // Local variables
    SX1278 *_radio;
    int _mode;
    uint8_t _spreadingFactor;
    uint32_t _frequency;
    int _txPower;
    uint32_t _spiFrequency;
    float _bandwidth;
    uint8_t _codingRate;
    uint8_t _syncWord; //SX127X_SYNC_WORD,
    uint8_t _currentLimit;
    uint8_t _preambleLength;
    uint8_t _gain;
    bool _transmitFlag = false;
};
