#pragma once

#include <RadioLib.h>

class RadioInterface
{
private:
    static const int PACKET_SIZE = 60; //in bytes
    Si446x radio=nullptr;
    QueueHandle_t txPacketsQueue = NULL;
    QueueHandle_t rxPacketsQueue = NULL;
    friend void vRadioTask(void *pvParameters);
    volatile bool receiveActive=false;
public:
    RadioInterface(int pin_cs, int pin_irq, int pin_sdn);
    ~RadioInterface();
    bool init();
    bool sendPacket(byte *data);
    bool receivePacket(byte *data);
    int getRxPacketLength();
    int getMaxPacketLength();
};
