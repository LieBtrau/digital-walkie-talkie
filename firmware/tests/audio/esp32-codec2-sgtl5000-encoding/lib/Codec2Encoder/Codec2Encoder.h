#pragma once
#include "SampleSource.h"
#include "codec2.h"

class Codec2Encoder : public SampleSink
{
private:
    int m_sample_rate = 8000; // Codec2 uses a fixed 8kHz sampling frequency
    CODEC2 *m_codec2;

public:
    Codec2Encoder(CODEC2 *codec2);
    virtual int sampleRate() { return m_sample_rate; }
    virtual void setFrames(QueueHandle_t inputQueue, byte* data, SemaphoreHandle_t xSemaphoreCodec2);
};