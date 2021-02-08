#pragma once

#include "I2SSampler.h"

class Sgtl5000Sampler : public I2SSampler
{
private:
    int m_sampleRate = 8000; //8kHz
    i2s_config_t m_i2s_config = {
		.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
		.sample_rate = m_sampleRate,
		.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
		.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // use left channel
		//I2S_CHANNEL_FMT_ALL_LEFT : WS toggles, L&R always get the same sample twice
		//I2S_CHANNEL_FMT_ONLY_LEFT : same as above
		.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
		.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1
		.dma_buf_count = 4,						  // number of buffers
		.dma_buf_len = I2SSampler::I2S_IN_DMA_BUFFER_LEN,
		.use_apll = true,
		.tx_desc_auto_clear = true,
		.fixed_mclk = m_sampleRate * 256};

protected:
    void configureI2S();

public:
    Sgtl5000Sampler(i2s_port_t i2sPort, i2s_pin_config_t* pin_config);
    void start(QueueHandle_t samplesQueue, int packetSize);
};
