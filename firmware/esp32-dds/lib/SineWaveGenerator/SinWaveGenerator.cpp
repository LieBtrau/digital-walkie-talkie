/* 
 */
#include <math.h>
#include "SinWaveGenerator.h"



SinWaveGenerator::SinWaveGenerator(int sample_rate, int frequency, float magnitude, float offset) : SampleSource(320)
{
    m_sample_rate = sample_rate;
    int bitwidth=sizeof(m_tuningWord)<<3;//number of bits in tuning_word
    m_tuningWord = pow(2,bitwidth) * frequency / m_sample_rate;
    //Sine wave table generated in Excel using the following formula, with indices [0..255]: =ROUND(SIN(2*PI()*A1/256)*127.5+127.5,0)
    for (int i = 0; i < 256; i++)
    {
        m_lut[i] = round(sin(M_TWOPI * i / 256) * magnitude + offset);
    }
}

void SinWaveGenerator::getFrames(Frame_t *frames, int number_frames)
{
    for (int i = 0; i < number_frames; i++)
    {
        m_phaseAccu+=m_tuningWord;
        frames[i].left = m_lut[m_phaseAccu>>8];
        frames[i].right = 0;
    }
    delay(15);
}
