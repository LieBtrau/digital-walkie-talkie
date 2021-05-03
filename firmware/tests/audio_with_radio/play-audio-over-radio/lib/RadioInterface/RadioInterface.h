#pragma once
/** 
 * Connections
 *  SI4463   	ESP32	Nucleo32
 *  VCC       	3V3		3V3
 * 	GND			GND		GND
 * 	MOSI		23		11
 * 	MISO		19		12
 * 	SCK			18		13
 * 	NSEL		5		A3
 *	IRQ			4		D3
 *	SDN			16		D6
 */
#include <RadioLib.h>

class RadioInterface
{
private:
    static const int PACKET_SIZE = 20; //in bytes
    Si446x radio = new Module(5, 4, 16);
    QueueHandle_t txPacketsQueue = NULL;
    QueueHandle_t rxPacketsQueue = NULL;
    friend void vRadioTask(void *pvParameters);

public:
    RadioInterface(/* args */);
    ~RadioInterface();
    bool init();
    bool sendPacket(byte *data);
    bool receivePacket(byte *data);
};
