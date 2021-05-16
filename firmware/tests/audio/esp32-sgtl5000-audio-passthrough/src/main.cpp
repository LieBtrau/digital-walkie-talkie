/**
 * Laptop     -> SGTL5000 -> ESP32 -> SGTL5000 -> head phones
 *          Analog out    I2S      I2S      Analog out                  
 * 
 * I2S peripheral works at full-duplex mode here, while ESP32-documentation mentions that only half-duplex is supported.
 * In this full duplex mode, it's not clear how and if the DMA-buffers are used.
 * 
 * Source code from : https://github.com/Jeija/esp32-lyrat-passthrough/blob/master/main/main.c
 * 
 * Signal name                              Adafruit 1780                   NodeMCU-32S
 * 3V3                                      3.3V                            3.3V
 * GND                                      GND                             GND
 *                                          G                               both ground pins of the Adafruit 1780 must be connected!
 * MCLK (Audio Master Clock)                23 (MCLK)                       GPIO0 (or another CLK_OUT pin)
 *      Sinusoidal signal, 1.8Vpp, 1.7Vavg.
 * LRCLK                                    20 (LRCLK)                      GPIO25 (or other GPIO)
 *      Audio Left/Right Clock
 *      WS (Word Select)
 *      48kHz square wave
 * BCLK                                     21 (BCLK)                       GPIO26 (or other GPIO)
 *      SCLK (Audio Bit Clock)
 *      1.54MHz
 * DOUT                                     8                               GPIO33 (or other GPIO)
 *      Audio Data from Audio Shield to Teensy
 * DIN                                      7                               GPIO23 (or other GPIO)
 *      Audio Data from MCU to Audio Shield
 * SDA                                      18                              GPIO21
 * SCL                                      19                              GPIO22


 */

#include "Arduino.h"
#include "driver/i2s.h"
#include "control_sgtl5000.h"
#include "pinconfig.h"

const i2s_port_t I2S_PORT = I2S_NUM_0;
const int I2S_READLEN = 256;
AudioControlSGTL5000 audioShield;
const int BUFLEN = 1000;
int16_t buffer[BUFLEN];
int bufCtr = 0;

void setup()
{
  Serial.begin(115200);
  int samplerate = 8000;

  // The I2S config as per the example
  const i2s_config_t i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX), // Receive, not transfer
      .sample_rate = samplerate,                                       // 8KHz
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // use left channel
      //I2S_CHANNEL_FMT_ALL_LEFT : WS toggles, L&R always get the same sample twice
      //I2S_CHANNEL_FMT_ONLY_LEFT : same as above
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1
      .dma_buf_count = 4,                       // number of buffers
      .dma_buf_len = I2S_READLEN,
      .use_apll = true,
      .tx_desc_auto_clear = true,
      .fixed_mclk = samplerate * 256};

  // The pin config as per the setup
  const i2s_pin_config_t pin_config = {
      .bck_io_num = PIN_BCLK,    // Serial Clock (SCK)
      .ws_io_num = PIN_LRCLK,    // Word Select (WS)
      .data_out_num = PIN_SDOUT, // data out to audio codec
      .data_in_num = PIN_SDIN    // data from audio codec
  };

  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
  ESP_ERROR_CHECK(i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL));
  ESP_ERROR_CHECK(i2s_set_pin(I2S_PORT, &pin_config));
  Serial.println("I2S driver installed.");
  // Enable MCLK output
  WRITE_PERI_REG(PIN_CTRL, READ_PERI_REG(PIN_CTRL) & 0xFFFFFFF0);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
  delay(5);
  Serial.printf("SGTL5000 %s initialized.\n", audioShield.enable() ? "is" : "not");
  //audioShield.lineInLevel(2); //2.22Vpp equals maximum output.
  audioShield.inputSelect(AUDIO_INPUT_MIC);
  audioShield.volume(0.2, 0.2);
  delay(200); //to skip the junk samples
}

int bufCnt = 0;
void loop()
{
  // Read a single sample and log it for the Serial Plotter.
  int16_t sample[I2S_READLEN / sizeof(int16_t)];
  size_t i2s_bytes_read = 0;
  size_t i2s_bytes_written = 0;

  /* continuously read data over I2S, pass it through and write it back */
  ESP_ERROR_CHECK(i2s_read(I2S_PORT, sample, I2S_READLEN, &i2s_bytes_read, portMAX_DELAY));
  ESP_ERROR_CHECK(i2s_write(I2S_PORT, sample, i2s_bytes_read, &i2s_bytes_written, portMAX_DELAY));
}