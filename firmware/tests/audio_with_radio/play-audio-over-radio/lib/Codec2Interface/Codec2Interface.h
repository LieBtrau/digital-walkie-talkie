#pragma once
#include "codec2.h"
#include "Arduino.h"

class Codec2Interface
{
private:
    static const int mode = CODEC2_MODE_1200;
    CODEC2 *codec2;
    QueueHandle_t xAudioSamplesQueue = NULL;
    QueueHandle_t xCodec2SamplesQueue = NULL;
    friend void codec2task(void *pvParameters);
    volatile bool codec2initOk = false;//volatile required for code to work
    int nsam;
    int nbyte;
    bool isEncoding = false;

public:
    Codec2Interface(/* args */);
    ~Codec2Interface();
    bool init();
    int getAudioSampleCount();
    int getCodec2PacketSize();
    bool startEncodingAudio(short *buf);
    bool getEncodedAudio(byte *bits);
    bool startDecodingAudio(byte *bits);
    bool getDecodedAudio(short *buf);
};
