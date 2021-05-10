#pragma once
#include "SampleSource.h"
#include "Codec2Interface.h"

class Codec2Decoder : public SampleSource
{
private:
    int m_sample_rate = 8000; // Codec2 uses a fixed 8kHz sampling frequency
    Codec2Interface *m_c2i = nullptr;

public:
    Codec2Decoder(Codec2Interface* c2i);
    bool init();
    virtual int sampleRate() { return m_sample_rate; }
    virtual void getFrames(Frame_t *frames, int &number_frames);
};