#pragma once

#include <Arduino.h>

/**
 * Base class for our sample consumers, single channel only
 **/
class SampleSink
{
public:
    SampleSink(){};
    SampleSink(int frameSize) : m_frameSampleCount(frameSize){};
    virtual int sampleRate() = 0;
    virtual void setFrames(int16_t *samples, int sampleCount) = 0;
    int getFrameSampleCount() { return m_frameSampleCount; };

protected:
    int m_frameSampleCount = 128;
};
