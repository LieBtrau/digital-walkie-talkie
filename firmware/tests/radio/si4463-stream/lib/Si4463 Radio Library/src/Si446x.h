/*
 * Project: Si4463 Radio Library for AVR and Arduino
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2017 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/si4463-radio-library-avr-arduino/
 */

#pragma once

#include <Arduino.h>
//#include <stdint.h>

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

class Si446x
{
public:
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
	* @brief Data structure for storing chip info from ::Si446x_getInfo()
	*/
	typedef struct
	{
		byte chipRev;	///< Chip revision
		word part;		///< Part ID
		byte partBuild; ///< Part build
		word id;		///< ID
		byte customer;	///< Customer
		byte romId;		///< ROM ID (3 = revB1B, 6 = revC2A)

		byte revExternal; ///< Revision external
		byte revBranch;	  ///< Revision branch
		byte revInternal; ///< Revision internal
		word patch;		  ///< Patch
		byte func;		  ///< Function
	} si446x_info_t;

	Si446x();
	void init(void);
	void getInfo(si446x_info_t *info);
	short getLatchedRSSI(void);
	short getRSSI(void);
	void setTxPower(byte pwr);
	void setupCallback(word callbacks, byte state);
	byte TX(byte *packet, byte len, byte channel, si446x_state_t onTxFinish);
	void RX(byte channel);
	void setLowBatt(word voltage);
	void setupWUT(byte r, word m, byte ldc, byte config);
	void disableWUT(void);
	byte sleep(void);
	word adc_gpio(byte pin);
	word adc_battery(void);
	float adc_temperature(void);
	void writeGPIO(si446x_gpio_t pin, byte value);
	byte readGPIO(void);
	byte dump(byte *buff, byte group);
	void read(byte *buff, byte len);
	void onReceive(void (*callback)(byte));
	void onReceiveBegin(void (*callback)(short));
	void onReceiveInvalid(void (*callback)(short));
	void onSent(void (*callback)(void));
	void onBatteryLow(void (*callback)(void));
	void onWakingUp(void (*callback)(void));

private:
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
	byte interrupt_off(void);
	byte interrupt_on(void);
	static void onIrqFalling(void);
	void handleIrqFall(void);
	byte getResponse(void *buff, byte len);
	byte waitForResponse(void *out, byte outLen, byte useTimeout);
	void irq_off(void);
	void irq_on(void);
	void doAPI(void *data, byte len, void *out, byte outLen);
	void setProperties(word prop, void *values, byte len);
	void setProperty(word prop, byte value);
	void getProperties(word prop, void *values, byte len);
	byte getProperty(word prop);
	word getADC(byte adc_en, byte adc_cfg, byte part);
	byte getFRR(byte reg);
	si446x_state_t getState(void);
	void setState(si446x_state_t newState);
	void clearFIFO(void);
	void interrupt(void *buff);
	void interrupt2(void *buff, byte clearPH, byte clearMODEM, byte clearCHIP);
	void resetDevice(void);
	void applyStartupConfig(void);
	void (*_onReceive)(byte) = nullptr;
	void (*_onReceiveBegin)(short) = nullptr;
	void (*_onReceiveInvalid)(short) = nullptr;
	void (*_onSent)(void) = nullptr;
	void (*_onBatteryLow)(void) = nullptr;
	void (*_onWakingUp)(void) = nullptr;
};