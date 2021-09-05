/*
 * Project: Si4463 Radio Library for AVR and Arduino
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2017 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/si4463-radio-library-avr-arduino/
 */

#pragma once

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <stdint.h>

#include "Si446x_config.h"

// Address matching doesnt really work very well as the FIFO still needs to be
// manually cleared after receiving a packet, so the MCU still needs to wakeup and
// do stuff instead of the radio doing things automatically :/
#if !DOXYGEN
#define SI446X_ENABLE_ADDRMATCHING 0
#endif

#define SI446X_MAX_PACKET_LEN 128 ///< Maximum packet length

#define SI446X_MAX_TX_POWER 127 ///< Maximum TX power (+20dBm/100mW)

#define SI446X_WUT_RUN 1  ///< Wake the microcontroller when the WUT expires
#define SI446X_WUT_BATT 2 ///< Take a battery measurement when the WUT expires
#define SI446X_WUT_RX 4	  ///< Go into RX mode for LDC time (not supported yet!)

#define SI446X_GPIO_PULL_EN 0x40  ///< Pullup enable for GPIO pins
#define SI446X_GPIO_PULL_DIS 0x00 ///< Pullup disable for GPIO pins
#define SI446X_NIRQ_PULL_EN 0x40  ///< Pullup enable for NIRQ pin
#define SI446X_NIRQ_PULL_DIS 0x00 ///< Pullup disable for NIRQ pin
#define SI446X_SDO_PULL_EN 0x40	  ///< Pullup enable for SDO pin
#define SI446X_SDO_PULL_DIS 0x00  ///< Pullup disable for SDO pin
#define SI446X_PIN_PULL_EN 0x40	  ///< Pullup enable for any pin
#define SI446X_PIN_PULL_DIS 0x00  ///< Pullup disable for any pin

#define SI446X_GPIO_DRV_HIGH 0x00	  ///< GPIO drive strength high
#define SI446X_GPIO_DRV_MED_HIGH 0x20 ///< GPIO drive strength medium-high
#define SI446X_GPIO_DRV_MED_LOW 0x40  ///< GPIO drive strength medium-low
#define SI446X_GPIO_DRV_LOW 0x60	  ///< GPIO drive strength low

#define SI446X_PROP_GROUP_GLOBAL 0x00		///< Property group global
#define SI446X_PROP_GROUP_INT 0x01			///< Property group interrupts
#define SI446X_PROP_GROUP_FRR 0x02			///< Property group fast response registers
#define SI446X_PROP_GROUP_PREAMBLE 0x10		///< Property group preamble
#define SI446X_PROP_GROUP_SYNC 0x11			///< Property group sync
#define SI446X_PROP_GROUP_PKT 0x12			///< Property group packet config
#define SI446X_PROP_GROUP_MODEM 0x20		///< Property group modem
#define SI446X_PROP_GROUP_MODEM_CHFLT 0x21	///< Property group RX coefficients
#define SI446X_PROP_GROUP_PA 0x22			///< Property group power amp
#define SI446X_PROP_GROUP_SYNTH 0x23		///< Property group synthesizer
#define SI446X_PROP_GROUP_MATCH 0x30		///< Property group address match
#define SI446X_PROP_GROUP_FREQ_CONTROL 0x40 ///< Property group frequency control
#define SI446X_PROP_GROUP_RX_HOP 0x50		///< Property group RX hop
#define SI446X_PROP_GROUP_PTI 0xF0			///< Property group packet trace interface

/**
* @brief GPIO pin modes (see the Si446x API docs for what they all mean)
*/
typedef enum
{
	SI446X_GPIO_MODE_DONOTHING = 0x00,
	SI446X_GPIO_MODE_TRISTATE = 0x01,
	SI446X_GPIO_MODE_DRIVE0 = 0x02,
	SI446X_GPIO_MODE_DRIVE1 = 0x03,
	SI446X_GPIO_MODE_INPUT = 0x04,
	SI446X_GPIO_MODE_32K_CLK = 0x05,
	SI446X_GPIO_MODE_BOOT_CLK = 0x06,
	SI446X_GPIO_MODE_DIV_CLK = 0x07,
	SI446X_GPIO_MODE_CTS = 0x08,
	SI446X_GPIO_MODE_INV_CTS = 0x09,
	SI446X_GPIO_MODE_CMD_OVERLAP = 0x0A,
	SI446X_GPIO_MODE_SDO = 0x0B,
	SI446X_GPIO_MODE_POR = 0x0C,
	SI446X_GPIO_MODE_CAL_WUT = 0x0D,
	SI446X_GPIO_MODE_WUT = 0x0E,
	SI446X_GPIO_MODE_EN_PA = 0x0F,
	SI446X_GPIO_MODE_TX_DATA_CLK = 0x10,
	SI446X_GPIO_MODE_RX_DATA_CLK = 0x11,
	SI446X_GPIO_MODE_EN_LNA = 0x12,
	SI446X_GPIO_MODE_TX_DATA = 0x13,
	SI446X_GPIO_MODE_RX_DATA = 0x14,
	SI446X_GPIO_MODE_RX_RAW_DATA = 0x15,
	SI446X_GPIO_MODE_ANTENNA_1_SW = 0x16,
	SI446X_GPIO_MODE_ANTENNA_2_SW = 0x17,
	SI446X_GPIO_MODE_VALID_PREAMBLE = 0x18,
	SI446X_GPIO_MODE_INVALID_PREAMBLE = 0x19,
	SI446X_GPIO_MODE_SYNC_WORD_DETECT = 0x1A,
	SI446X_GPIO_MODE_CCA = 0x1B,
	SI446X_GPIO_MODE_IN_SLEEP = 0x1C,
	SI446X_GPIO_MODE_PKT_TRACE = 0x1D,
	// Nothing for 0x1E (30)
	SI446X_GPIO_MODE_TX_RX_DATA_CLK = 0x1F,
	SI446X_GPIO_MODE_TX_STATE = 0x20,
	SI446X_GPIO_MODE_RX_STATE = 0x21,
	SI446X_GPIO_MODE_RX_FIFO_FULL = 0x22,
	SI446X_GPIO_MODE_TX_FIFO_EMPTY = 0x23,
	SI446X_GPIO_MODE_LOW_BATT = 0x24,
	SI446X_GPIO_MODE_CCA_LATCH = 0x25,
	SI446X_GPIO_MODE_HOPPED = 0x26,
	SI446X_GPIO_MODE_HOP_TABLE_WRAP = 0x27
} si446x_gpio_mode_t;

/**
* @brief NIRQ pin modes (see the Si446x API docs for what they all mean)
*/
typedef enum
{
	SI446X_NIRQ_MODE_DONOTHING = 0x00,
	SI446X_NIRQ_MODE_TRISTATE = 0x01,
	SI446X_NIRQ_MODE_DRIVE0 = 0x02,
	SI446X_NIRQ_MODE_DRIVE1 = 0x03,
	SI446X_NIRQ_MODE_INPUT = 0x04,
	//	SI446X_NIRQ_MODE_32K_CLK	= 0x05,
	//	SI446X_NIRQ_MODE_BOOT_CLK	= 0x06,
	SI446X_NIRQ_MODE_DIV_CLK = 0x07,
	SI446X_NIRQ_MODE_CTS = 0x08,
	//	SI446X_NIRQ_MODE_INV_CTS	= 0x09,
	//	SI446X_NIRQ_MODE_CMD_OVERLAP	= 0x0A,
	SI446X_NIRQ_MODE_SDO = 0x0B,
	SI446X_NIRQ_MODE_POR = 0x0C,
	//	SI446X_NIRQ_MODE_CAL_WUT	= 0x0D,
	//	SI446X_NIRQ_MODE_WUT		= 0x0E,
	SI446X_NIRQ_MODE_EN_PA = 0x0F,
	SI446X_NIRQ_MODE_TX_DATA_CLK = 0x10,
	SI446X_NIRQ_MODE_RX_DATA_CLK = 0x11,
	SI446X_NIRQ_MODE_EN_LNA = 0x12,
	SI446X_NIRQ_MODE_TX_DATA = 0x13,
	SI446X_NIRQ_MODE_RX_DATA = 0x14,
	SI446X_NIRQ_MODE_RX_RAW_DATA = 0x15,
	SI446X_NIRQ_MODE_ANTENNA_1_SW = 0x16,
	SI446X_NIRQ_MODE_ANTENNA_2_SW = 0x17,
	SI446X_NIRQ_MODE_VALID_PREAMBLE = 0x18,
	SI446X_NIRQ_MODE_INVALID_PREAMBLE = 0x19,
	SI446X_NIRQ_MODE_SYNC_WORD_DETECT = 0x1A,
	SI446X_NIRQ_MODE_CCA = 0x1B,
	//	SI446X_NIRQ_MODE_IN_SLEEP		= 0x1C,
	SI446X_NIRQ_MODE_PKT_TRACE = 0x1D,
	// Nothing for 0x1E (30)
	SI446X_NIRQ_MODE_TX_RX_DATA_CLK = 0x1F,
	//	SI446X_NIRQ_MODE_TX_STATE		= 0x20,
	//	SI446X_NIRQ_MODE_RX_STATE		= 0x21,
	//	SI446X_NIRQ_MODE_RX_FIFO_FULL	= 0x22,
	//	SI446X_NIRQ_MODE_TX_FIFO_EMPTY	= 0x23,
	//	SI446X_NIRQ_MODE_LOW_BATT		= 0x24,
	//	SI446X_NIRQ_MODE_CCA_LATCH		= 0x25,
	//	SI446X_NIRQ_MODE_HOPPED			= 0x26,
	SI446X_NIRQ_MODE_NIRQ = 0x27
} si446x_nirq_mode_t;

/**
* @brief SDO pin modes (see the Si446x API docs for what they all mean)
*/
typedef enum
{
	SI446X_SDO_MODE_DONOTHING = 0x00,
	SI446X_SDO_MODE_TRISTATE = 0x01,
	SI446X_SDO_MODE_DRIVE0 = 0x02,
	SI446X_SDO_MODE_DRIVE1 = 0x03,
	SI446X_SDO_MODE_INPUT = 0x04,
	SI446X_SDO_MODE_32K_CLK = 0x05,
	//	SI446X_SDO_MODE_BOOT_CLK	= 0x06,
	SI446X_SDO_MODE_DIV_CLK = 0x07,
	SI446X_SDO_MODE_CTS = 0x08,
	//	SI446X_SDO_MODE_INV_CTS	= 0x09,
	//	SI446X_SDO_MODE_CMD_OVERLAP	= 0x0A,
	SI446X_SDO_MODE_SDO = 0x0B,
	SI446X_SDO_MODE_POR = 0x0C,
	//	SI446X_SDO_MODE_CAL_WUT	= 0x0D,
	SI446X_SDO_MODE_WUT = 0x0E,
	SI446X_SDO_MODE_EN_PA = 0x0F,
	SI446X_SDO_MODE_TX_DATA_CLK = 0x10,
	SI446X_SDO_MODE_RX_DATA_CLK = 0x11,
	SI446X_SDO_MODE_EN_LNA = 0x12,
	SI446X_SDO_MODE_TX_DATA = 0x13,
	SI446X_SDO_MODE_RX_DATA = 0x14,
	SI446X_SDO_MODE_RX_RAW_DATA = 0x15,
	SI446X_SDO_MODE_ANTENNA_1_SW = 0x16,
	SI446X_SDO_MODE_ANTENNA_2_SW = 0x17,
	SI446X_SDO_MODE_VALID_PREAMBLE = 0x18,
	SI446X_SDO_MODE_INVALID_PREAMBLE = 0x19,
	SI446X_SDO_MODE_SYNC_WORD_DETECT = 0x1A,
	SI446X_SDO_MODE_CCA = 0x1B,
	//	SI446X_SDO_MODE_IN_SLEEP		= 0x1C,
	//	SI446X_SDO_MODE_PKT_TRACE		= 0x1D,
	// Nothing for 0x1E (30)
	//	SI446X_SDO_MODE_TX_RX_DATA_CLK	= 0x1F,
	//	SI446X_SDO_MODE_TX_STATE		= 0x20,
	//	SI446X_SDO_MODE_RX_STATE		= 0x21,
	//	SI446X_SDO_MODE_RX_FIFO_FULL	= 0x22,
	//	SI446X_SDO_MODE_TX_FIFO_EMPTY	= 0x23,
	//	SI446X_SDO_MODE_LOW_BATT		= 0x24,
	//	SI446X_SDO_MODE_CCA_LATCH		= 0x25,
	//	SI446X_SDO_MODE_HOPPED			= 0x26,
	//	SI446X_SDO_MODE_HOP_TABLE_WRAP	= 0x27
} si446x_sdo_mode_t;

/**
* @brief Data structure for storing chip info from ::Si446x_getInfo()
*/
typedef struct
{
	uint8_t chipRev;   ///< Chip revision
	uint16_t part;	   ///< Part ID
	uint8_t partBuild; ///< Part build
	uint16_t id;	   ///< ID
	uint8_t customer;  ///< Customer
	uint8_t romId;	   ///< ROM ID (3 = revB1B, 6 = revC2A)

	uint8_t revExternal; ///< Revision external
	uint8_t revBranch;	 ///< Revision branch
	uint8_t revInternal; ///< Revision internal
	uint16_t patch;		 ///< Patch
	uint8_t func;		 ///< Function
} si446x_info_t;

/**
* @brief GPIOs for passing to ::Si446x_writeGPIO(), or for masking when reading from ::Si446x_readGPIO()
*/
typedef enum
{
	SI446X_GPIO0 = 0, ///< GPIO 1
	SI446X_GPIO1 = 1, ///< GPIO 2
	SI446X_GPIO2 = 2, ///< GPIO 3
	SI446X_GPIO3 = 3, ///< GPIO 4
	SI446X_NIRQ = 4,  ///< NIRQ
	SI446X_SDO = 5	  ///< SDO
} si446x_gpio_t;

/**
* @brief Radio states, returned from ::Si446x_getState()
*/
typedef enum
{
	SI446X_STATE_NOCHANGE = 0x00,
	SI446X_STATE_SLEEP = 0x01, ///< This will never be returned since SPI activity will wake the radio into ::SI446X_STATE_SPI_ACTIVE
	SI446X_STATE_SPI_ACTIVE = 0x02,
	SI446X_STATE_READY = 0x03,
	SI446X_STATE_READY2 = 0x04,	 ///< Will return as ::SI446X_STATE_READY
	SI446X_STATE_TX_TUNE = 0x05, ///< Will return as ::SI446X_STATE_TX
	SI446X_STATE_RX_TUNE = 0x06, ///< Will return as ::SI446X_STATE_RX
	SI446X_STATE_TX = 0x07,
	SI446X_STATE_RX = 0x08
} si446x_state_t;

#define SI446X_CBS_SENT _BV(5 + 8) ///< Enable/disable packet sent callback
//#define SI446X_CBS_RXCOMPLETE		_BV(4+8)
//#define SI446X_CBS_RXINVALID		_BV(3+8)
#define SI446X_CBS_RXBEGIN _BV(0) ///< Enable/disable packet receive begin callback
//#define SI446X_CBS_INVALIDSYNC		_BV(5) ///< Don't use this, it's used internally by the library

class Si446x
{
public:
	Si446x();
	void Si446x_init(void);
	void Si446x_getInfo(si446x_info_t *info);
	int16_t Si446x_getRSSI(void);
	void Si446x_setTxPower(uint8_t pwr);
	void Si446x_setupCallback(uint16_t callbacks, uint8_t state);
	uint8_t Si446x_TX(void *packet, uint8_t len, uint8_t channel, si446x_state_t onTxFinish);
	void Si446x_RX(uint8_t channel);
	void Si446x_setLowBatt(uint16_t voltage);
	void Si446x_setupWUT(uint8_t r, uint16_t m, uint8_t ldc, uint8_t config);
	void Si446x_disableWUT(void);
	uint8_t Si446x_sleep(void);
	si446x_state_t Si446x_getState(void);
	uint16_t Si446x_adc_gpio(uint8_t pin);
	uint16_t Si446x_adc_battery(void);
	float Si446x_adc_temperature(void);
	void Si446x_writeGPIO(si446x_gpio_t pin, uint8_t value);
	uint8_t Si446x_readGPIO(void);
	uint8_t Si446x_dump(void *buff, uint8_t group);
	void Si446x_read(void *buff, uint8_t len);

private:
	static void Si446x_SERVICE(void);
	void handleInterrupt(void);
	uint8_t Si446x_irq_off(void);
	void Si446x_irq_on(uint8_t origVal);
	void doAPI(void *data, uint8_t len, void *out, uint8_t outLen);
	void setProperties(uint16_t prop, void *values, uint8_t len);
	void setProperty(uint16_t prop, uint8_t value);
	void getProperties(uint16_t prop, void *values, uint8_t len);
	uint8_t getProperty(uint16_t prop);
	uint16_t getADC(uint8_t adc_en, uint8_t adc_cfg, uint8_t part);
	void setState(si446x_state_t newState);
	void clearFIFO(void);
	void interrupt(void *buff);
	void interrupt2(void *buff, uint8_t clearPH, uint8_t clearMODEM, uint8_t clearCHIP);
	void applyStartupConfig(void);
};