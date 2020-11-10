#include "DacOutput.h"
#include "SampleSource.h"

void DacOutput::start(SampleSource *sample_generator)
{
    // i2s config for writing both channels of I2S
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
        .sample_rate = sample_generator->sampleRate(),
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,   //the DAC module will only take the 8bits from MSB
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false};
    I2SOutput::start(i2sConfig, sample_generator);
}