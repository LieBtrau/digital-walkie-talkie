#pragma once
#include "SampleSource.h"
#include "codec2.h"

class Codec2Generator : public SampleSource
{
private:
    int m_sample_rate = 8000; // Codec2 uses a fixed 8kHz sampling frequency
    CODEC2 *m_codec2;
    QueueHandle_t m_xCodec2DataQueue;
    int nsam, nbit, nbyte;
    short *buf;
    unsigned char *bits;

public:
    Codec2Generator(CODEC2 *codec2, QueueHandle_t xCodec2DataQueue);
    virtual int sampleRate() { return m_sample_rate; }
    virtual void getFrames(Frame_t *frames, int number_frames, SemaphoreHandle_t xSemaphoreCodec2);
};