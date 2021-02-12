/* 
 */
#include <math.h>
#include "Codec2Decoder.h"

Codec2Generator::Codec2Generator(CODEC2 *codec2) : SampleSource(codec2_samples_per_frame(codec2)),
                                                                              m_codec2(codec2)
{
    nsam = getFrameSampleCount();
    nbit = codec2_bits_per_frame(codec2);
    buf = (short *)malloc(nsam * sizeof(short));
    nbyte = (nbit + 7) / 8;
}

/* This function should not take longer than codec2_samples_per_frame / sample rate
 * e.g. for codec2 1200bps: 320 / 8000 = 40ms
 */
void Codec2Generator::getFrames(byte *bits, QueueHandle_t outputQueue, SemaphoreHandle_t xSemaphoreCodec2)
{
    Frame_t samples[getFrameSampleCount()];
    if (xSemaphoreTake(xSemaphoreCodec2, (TickType_t)10) == pdTRUE)
    {
        codec2_decode(m_codec2, buf, bits);
        for (int i = 0; i < nsam; i++)
        {
            //For internal audio DAC
            // int audio = buf[i] * m_magnitude + 32768;                //convert from signed to unsigned 16bit
            // audio = audio < 0 ? 0 : (audio > 65535 ? 65535 : audio); //audio clipping
            // frames[i].left = audio;

            //For external audio DAC
            samples[i].left = buf[i];

            samples[i].right = 0;
        }
        /* We have finished accessing the shared resource.  Release the semaphore. */
        xSemaphoreGive(xSemaphoreCodec2);
        xQueueSendToBack(outputQueue, samples, portMAX_DELAY);
    }
}
