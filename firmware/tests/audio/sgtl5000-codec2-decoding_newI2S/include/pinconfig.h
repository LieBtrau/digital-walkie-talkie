#include "Arduino.h"
/**
 * Audio codec connections
 * -----------------------
 * Signal name                              Adafruit 1780                   NodeMCU-32S
 * -----------------------------------------------------------------------------------------------------------------
 * 3V3                                      3.3V                            3.3V
 * GND                                      GND                             GND
 *                                          G                               both ground pins of the Adafruit 1780 must be connected!
 * MCLK (Audio Master Clock)                23 (MCLK)                       GPIO0 (or another CLK_OUT pin)
 *      2MHz, Sinusoidal signal, 1.8Vpp, 1.7Vavg.
 * LRCLK                                    20 (LRCLK)                      */const int PIN_LRCLK = 25;/*
 *      Audio Left/Right Clock
 *      WS (Word Select)
 *      8kHz square wave
 * BCLK                                     21 (BCLK)                       */const int PIN_BCLK = 26;/*
 *      SCLK (Audio Bit Clock)
 *      256kHz
 * DOUT                                     8                               */const int PIN_SDIN = 33;/*
 *      Audio Data from Audio Shield to Teensy/ESP32
 * DIN                                      7                               */const int PIN_SDOUT = 32;/*
 *      Audio Data from MCU to Audio Shield
 * SDA                                      18                              GPIO21
 * SCL                                      19                              GPIO22
 * 
 * Radio connections
 * -----------------
 * Signal name                              SI4463							NodeMCU-32S
 * --------------------------------------------------------------------------------------------------------------
 * Power in                                 VCC                          	3V3
 * Ground									GND								GND
 * SPI MOSI									MOSI							GPIO23
 * SPI MISO									MISO							GPIO19
 * SPI SCLK									SCK								GPIO18
 * SPI /CS									NSEL							*/const int PIN_CS = 5;/*
 * /IRQ										IRQ								*/const int PIN_IRQ = 4;/*
 * Shutdown									SDN								*/const int PIN_SDN = 16;/*
 */