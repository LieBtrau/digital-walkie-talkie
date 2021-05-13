#include "Codec2Encoder.h"

Codec2Encoder::Codec2Encoder(Codec2Interface *c2i) : m_c2i(c2i)
{
}

bool Codec2Encoder::init()
{
    if (!m_c2i->init())
    {
        Serial.println("Can't init codec2");
        return false;
    }
    m_frameSampleCount = m_c2i->getAudioSampleCount();

    return true;
}

void Codec2Encoder::setFrames(int16_t *samples, int sampleCount)
{
    if (sampleCount == m_frameSampleCount)
    {
        m_c2i->startEncodingAudio(samples);
    }
}