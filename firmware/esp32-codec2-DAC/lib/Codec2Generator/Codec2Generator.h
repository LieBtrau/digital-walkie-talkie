#pragma once
#include "SampleSource.h"
#include "codec2.h"

class Codec2Generator : public SampleSource
{
private:
    int m_sample_rate = 8000;       // Codec2 uses a fixed 8kHz sampling frequency
    CODEC2* m_codec2;
    byte* m_bitSource;
    byte m_magnitude;
public:
    Codec2Generator(CODEC2* codec2, byte* bitSource, byte magnitude);
    virtual int sampleRate() { return m_sample_rate; }
    virtual void getFrames(Frame_t *frames, int number_frames);
};