#include "Codec2Interface.h"

static Codec2Interface *ci;
/**
 * Because of the 8K task size limitation of FreeRTOS on the ESP32, we must create the codec2 object
 * inside a task that we give at least 24K of memory
 */
void codec2task(void *pvParameters)
{
    ci->codec2 = codec2_create(ci->mode);
    codec2_set_natural_or_gray(ci->codec2, 0);

    ci->nsam = codec2_samples_per_frame(ci->codec2);
    ci->xEncoderAudioIn = xQueueCreate(3, sizeof(int16_t) * ci->nsam);
    ci->xDecoderAudioOut = xQueueCreate(3, sizeof(int16_t) * ci->nsam);
    if (ci->xEncoderAudioIn == NULL || ci->xDecoderAudioOut == NULL)
    {
        Serial.println("Can't create xAudioSamplesQueue");
        while (true)
            ;
    }

    ci->nbyte = (codec2_bits_per_frame(ci->codec2) + 7) / 8;
    ci->xEncoderCodec2Out = xQueueCreate(3, ci->nbyte);
    ci->xDecoderCodec2In = xQueueCreate(3, ci->nbyte);
    if (ci->xEncoderCodec2Out == NULL || ci->xDecoderCodec2In == NULL)
    {
        Serial.println("Can't create xCodec2SamplesQueue");
        while (true)
            ;
    }

    short *buf = (short *)malloc(ci->nsam * sizeof(short));
    unsigned char *bits = (unsigned char *)malloc(ci->nbyte * sizeof(char));
    if (bits != nullptr && buf != nullptr)
    {
        ci->codec2initOk = true;
    }
    else
    {
        while (true)
            ;
    }
    for (;;)
    {
        //Code crashes when 0 is filled in for xQueueReceive xTicksToWait
        if (ci->isEncoding)
        {
            //Codec2 encoding : Receive from audio queue and send to codec2 queue
            if (xQueueReceive(ci->xEncoderAudioIn, buf, 1) == pdTRUE)
            {
                codec2_encode(ci->codec2, bits, buf);
                xQueueSendToBack(ci->xEncoderCodec2Out, bits, 1);
            }
        }
        else
        {
            //Codec2 decoding : Receive from codec2 bits and send to audio queue
            if (xQueueReceive(ci->xDecoderCodec2In, bits, 100) == pdTRUE)
            {
                codec2_decode(ci->codec2, buf, bits);
                xQueueSendToBack(ci->xDecoderAudioOut, buf, 100);
            }
        }
    }
}

Codec2Interface::Codec2Interface(/* args */)
{
    ci = this;
}

Codec2Interface::~Codec2Interface()
{
    codec2_destroy(ci->codec2);
}

bool Codec2Interface::init()
{
    if (!codec2initOk)
    {
        xTaskCreate(codec2task, "Codec2Task", 24576, NULL, 2, NULL);
        while (!codec2initOk)
            ;
    }
    return true;
}

int Codec2Interface::getAudioSampleCount()
{
    while (!codec2initOk)
        ;
    return nsam;
}

int Codec2Interface::getCodec2PacketSize()
{
    while (!codec2initOk)
        ;
    return nbyte;
}

bool Codec2Interface::startEncodingAudio(short *buf)
{
    isEncoding = true;
    return xQueueSendToBack(xEncoderAudioIn, buf, (TickType_t)1000) == pdTRUE;
}

bool Codec2Interface::getEncodedAudio(byte *bits)
{
    return xQueueReceive(xEncoderCodec2Out, bits, 1000) == pdTRUE;
}

bool Codec2Interface::isAvailableEncodedFrame()
{
    return uxQueueMessagesWaiting(xEncoderCodec2Out) > 0;
}

bool Codec2Interface::isDecodingInputBufferSpaceLeft()
{
    return uxQueueSpacesAvailable(xDecoderCodec2In) > 0;
}

bool Codec2Interface::startDecodingAudio(byte *bits)
{
    isEncoding = false;
    return xQueueSendToBack(xDecoderCodec2In, bits, (TickType_t)1000) == pdTRUE;
}

bool Codec2Interface::getDecodedAudio(short *buf)
{
    return xQueueReceive(xDecoderAudioOut, buf, 1000) == pdTRUE;
}
