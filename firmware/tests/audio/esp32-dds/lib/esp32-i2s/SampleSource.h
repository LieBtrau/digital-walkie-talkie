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
    SampleSource(int frameSize) : m_frameSampleCount(frameSize){};
    virtual int sampleRate() = 0;
    virtual void getFrames(byte* data, QueueHandle_t outputQueue, SemaphoreHandle_t xSemaphoreCodec2) = 0;
    int getFrameSampleCount(){return m_frameSampleCount;};
private:
    int m_frameSampleCount = 128;
};

/**
 * Base class for our sample consumers
 **/
class SampleSink
{
public:
    SampleSink(){};
    SampleSink(int frameSize) : m_frameSampleCount(frameSize){};
    virtual int sampleRate() = 0;
    virtual void setFrames(QueueHandle_t inputQueue, byte* data, SemaphoreHandle_t xSemaphoreCodec2) = 0;
    int getFrameSampleCount(){return m_frameSampleCount;};
private:
    int m_frameSampleCount = 128;
};

