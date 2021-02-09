#pragma once
#include "SampleSource.h"
#include "codec2.h"

class Codec2Generator : public SampleSource
{
private:
    int m_sample_rate = 8000; // Codec2 uses a fixed 8kHz sampling frequency
    CODEC2 *m_codec2;
    int nsam, nbit, nbyte;
    short *buf;

public:
    Codec2Generator(CODEC2 *codec2);
    virtual int sampleRate() { return m_sample_rate; }
    virtual void getFrames(byte* data, QueueHandle_t outputQueue, SemaphoreHandle_t xSemaphoreCodec2);
};