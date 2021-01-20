/* 
 */
#include <math.h>
#include "Codec2Generator.h"

int nsam, nbit, nbyte;
short *buf;
unsigned char *bits;
int nbit_ctr = 0;

Codec2Generator::Codec2Generator(CODEC2 *codec2, byte *bitSource, unsigned int sourceByteCount, byte magnitude) : SampleSource(codec2_samples_per_frame(codec2)),
                                                                                    m_codec2(codec2),
                                                                                    m_bitSource(bitSource),
                                                                                    m_bitSourceByteCount(sourceByteCount),
                                                                                    m_magnitude(magnitude)
{
    nsam = codec2_samples_per_frame(codec2);
    nbit = codec2_bits_per_frame(codec2);
    buf = (short *)malloc(nsam * sizeof(short));
    nbyte = (nbit + 7) / 8;
    bits = (unsigned char *)malloc(nbyte * sizeof(char));
}

/* This function should not take longer than codec2_samples_per_frame / sample rate
 * e.g. for codec2 1200bps: 320 / 8000 = 40ms
 */
void Codec2Generator::getFrames(Frame_t *frames, int number_frames)
{
    if (nbit_ctr + nbyte < m_bitSourceByteCount)
    {
        memcpy(bits, m_bitSource + nbit_ctr, nbyte);
        codec2_decode(m_codec2, buf, bits);
        for (int i = 0; i < nsam; i++)
        {
            //For internal audio DAC
            // int audio = buf[i] * m_magnitude + 32768;                //convert from signed to unsigned 16bit
            // audio = audio < 0 ? 0 : (audio > 65535 ? 65535 : audio); //audio clipping
            // frames[i].left = audio;

            //For external audio DAC
            frames[i].left = buf[i];
            
            frames[i].right = 0;
        }
        nbit_ctr += nbyte;
    }
    else
    {
        nbit_ctr = 0;
    }
}
