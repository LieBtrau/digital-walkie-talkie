#include "Sgtl5000Sampler.h"

Sgtl5000Sampler::Sgtl5000Sampler(i2s_port_t i2sPort, i2s_pin_config_t pin_config) : _i2s_port(i2sPort),
																					m_pin_config(pin_config)
{
}

void Sgtl5000Sampler::configureI2S()
{
	ESP_ERROR_CHECK(i2s_set_pin(getI2SPort(), &m_pin_config));
	// Enable MCLK output
	WRITE_PERI_REG(PIN_CTRL, READ_PERI_REG(PIN_CTRL) & 0xFFFFFFF0);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
	delay(5);
}

void Sgtl5000Sampler::start(QueueHandle_t samplesQueue, int packetSize)
{
	i2s_config_t i2s_config = {
		.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
		.sample_rate = _sampleRate,
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
		.fixed_mclk = _sampleRate * 256};

	I2SSampler::start(_i2s_port, i2s_config, samplesQueue, packetSize);
}