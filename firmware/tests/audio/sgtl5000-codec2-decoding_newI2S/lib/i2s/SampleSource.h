#pragma once

#include <Arduino.h>

typedef struct
{
    int16_t left;
    int16_t right;
} Frame_t;

/**
 * Base class for our sample generators
 **/
class SampleSource
{
public:
    SampleSource(){};
    virtual int sampleRate() = 0;
    // This should fill the samples buffer with the specified number of frames
    // A frame contains a LEFT and a RIGHT sample. Each sample should be signed 16 bits
    virtual void getFrames(Frame_t *frames, int &number_frames) = 0;
    int getFrameSampleCount() { return m_frameSampleCount; };

protected:
    int m_frameSampleCount = 128;
};