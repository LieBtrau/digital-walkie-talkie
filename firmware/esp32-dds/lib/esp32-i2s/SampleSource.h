#pragma once

#include "Arduino.h"

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
    virtual void getFrames(byte* data, QueueHandle_t outputQueue, SemaphoreHandle_t xSemaphoreCodec2) = 0;
    int getFrameSize(){return m_frameSize;};
private:
    int m_frameSize = 128;
};