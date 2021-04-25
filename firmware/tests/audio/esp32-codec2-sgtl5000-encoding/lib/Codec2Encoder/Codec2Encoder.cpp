/* 
 */
#include <math.h>
#include "Codec2Encoder.h"

Codec2Encoder::Codec2Encoder(CODEC2 *codec2) : SampleSink(codec2_samples_per_frame(codec2)),
                                               m_codec2(codec2)
{
}

/* This function should not take longer than codec2_samples_per_frame / sample rate
 * e.g. for codec2 1200bps: 320 / 8000 = 40ms
 */
void Codec2Encoder::setFrames(QueueHandle_t inputQueue, byte *bits, SemaphoreHandle_t xSemaphoreCodec2)
{
    short samples[codec2_samples_per_frame(m_codec2)];
    if (xQueueReceive(inputQueue, samples, portMAX_DELAY) == pdTRUE && xSemaphoreCodec2 != NULL)
    {
        /* See if we can obtain the semaphore.  If the semaphore is not available wait 10 ticks to see if it becomes free. */
        if (xSemaphoreTake(xSemaphoreCodec2, (TickType_t)10) == pdTRUE)
        {
            /* We were able to obtain the semaphore and can now access the shared resource. */
            codec2_encode(m_codec2, bits, samples);
            /* We have finished accessing the shared resource.  Release the semaphore. */
            xSemaphoreGive(xSemaphoreCodec2);
        }
    }
}
