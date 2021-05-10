#include "Codec2Decoder.h"

Codec2Decoder::Codec2Decoder(Codec2Interface *c2i) : m_c2i(c2i)
{
}

bool Codec2Decoder::init()
{
    if (!m_c2i->init())
    {
        Serial.println("Can't init codec2");
        return false;
    }
    m_frameSampleCount = m_c2i->getAudioSampleCount();

    return true;
}

void Codec2Decoder::getFrames(Frame_t *frames, int &number_frames)
{
    short buf[m_frameSampleCount];
    number_frames = 0;
    if (m_c2i->getDecodedAudio(buf))
    {
        for (int i = 0; i < m_frameSampleCount; i++)
        {
            //For internal audio DAC
            // int audio = buf[i] * m_magnitude + 32768;                //convert from signed to unsigned 16bit
            // audio = audio < 0 ? 0 : (audio > 65535 ? 65535 : audio); //audio clipping
            // frames[i].left = audio;

            //For external audio DAC
            frames[i].left = buf[i];
            frames[i].right = 0;
        }
        number_frames = m_frameSampleCount;
    }
}
