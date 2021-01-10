/**
 * The SGTL5000 treats the 16bit data as 16bit signed.
 * 
 * 0x7FFF : 32767 : Vout minimum
 * 0x0000 : flat line 25mV
 * 0x8000 : -32768 : Vout maximum
 */

#include "Sgtl5000_Output.h"
#include "SampleSource.h"

Sgtl5000_Output::Sgtl5000_Output(byte pin_SCK, byte pin_WS, byte pin_DOUT) : _pin_SCK(pin_SCK), _pin_WS(pin_WS), _pin_DOUT(pin_DOUT)
{
}

void Sgtl5000_Output::start(SampleSource *sample_generator)
{
    static i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX), // Only TX
        .sample_rate = sample_generator->sampleRate(),       // Default: 48kHz
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,        // 16-bit per channel
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,        // 2-channels
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, //Interrupt level 1
        .dma_buf_count = 32,
        .dma_buf_len = 64, //
        .use_apll = true,
        .tx_desc_auto_clear = true,
        .fixed_mclk = sample_generator->sampleRate() * 256
        };

    static i2s_pin_config_t i2s_pin_config = {
        .bck_io_num = _pin_SCK,
        .ws_io_num = _pin_WS,
        .data_out_num = _pin_DOUT};

    I2SOutput::start(I2S_NUM_0, i2s_pin_config, i2sConfig, sample_generator);

    // Enable MCLK output
    WRITE_PERI_REG(PIN_CTRL, READ_PERI_REG(PIN_CTRL)&0xFFFFFFF0);
    PIN_FUNC_SELECT (PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
}