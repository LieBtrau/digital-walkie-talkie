/* 
 */
#include <math.h>
#include "Codec2Generator.h"



Codec2Generator::Codec2Generator(CODEC2 *codec2, QueueHandle_t xCodec2DataQueue) : SampleSource(codec2_samples_per_frame(codec2)),
                                                                                    m_codec2(codec2),
                                                                                    m_xCodec2DataQueue(xCodec2DataQueue)
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
    if(xQueueReceive(m_xCodec2DataQueue, bits, portMAX_DELAY) == pdTRUE)
    {
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
    }
}
