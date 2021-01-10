#pragma once

#include <Arduino.h>


typedef struct
{
    uint16_t left;
    uint16_t right;
} Frame_t;

/**
 * Base class for our sample generators
 **/
class SampleSource
{
public:
    SampleSource(){};
    SampleSource(int frameSize) : m_frameSize(frameSize){};
    virtual int sampleRate() = 0;
    // This should fill the samples buffer with the specified number of frames
    // A frame contains a LEFT and a RIGHT sample.
    // This function should take no longer than m_frameSize / sampleRate
    virtual void getFrames(Frame_t *frames, int number_frames) = 0;
    int getFrameSize(){return m_frameSize;};
private:
    int m_frameSize = 128;
};