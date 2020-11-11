#ifndef __sin_wave_generator_h__
#define __sin_wave_generator_h__

#include "SampleSource.h"

class SinWaveGenerator : public SampleSource
{
private:
    int m_sample_rate;
    int m_frequency;
    byte m_magnitude;
    uint16_t m_phaseAccu;
    uint16_t m_tuningWord;
    byte m_lut[256];

public:
    SinWaveGenerator(int sample_rate, int frequency, byte magnitude);
    virtual int sampleRate() { return m_sample_rate; }
    virtual void getFrames(Frame_t *frames, int number_frames);
};

#endif