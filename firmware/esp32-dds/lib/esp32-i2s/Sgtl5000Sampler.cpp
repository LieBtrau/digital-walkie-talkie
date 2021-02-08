#include "Sgtl5000Sampler.h"

Sgtl5000Sampler::Sgtl5000Sampler(i2s_port_t i2sPort, i2s_pin_config_t* pin_config) : I2SSampler(i2sPort, pin_config)
{
}

void Sgtl5000Sampler::configureI2S()
{
	// Enable MCLK output
	WRITE_PERI_REG(PIN_CTRL, READ_PERI_REG(PIN_CTRL) & 0xFFFFFFF0);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
	delay(5);
}

void Sgtl5000Sampler::start(QueueHandle_t samplesQueue, int packetSize)
{

	I2SSampler::start(&m_i2s_config, samplesQueue, packetSize);
}