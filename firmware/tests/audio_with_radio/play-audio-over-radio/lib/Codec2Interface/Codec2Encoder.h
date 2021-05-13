#pragma once
#include "SampleSink.h"
#include "Codec2Interface.h"

class Codec2Encoder : public SampleSink
{
private:
    int m_sample_rate = 8000; // Codec2 uses a fixed 8kHz sampling frequency
    Codec2Interface *m_c2i = nullptr;

public:
    Codec2Encoder(Codec2Interface* c2i);
    bool init();
    virtual int sampleRate() { return m_sample_rate; }
    virtual void setFrames(int16_t* samples, int sampleCount);
};