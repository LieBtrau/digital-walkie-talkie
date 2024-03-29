/* 
 * Project: Si4463 Radio Library for AVR and Arduino
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Changes : Christoph Tack 
 * Copyright: (C) 2017 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/si4463-radio-library-avr-arduino/
 */


#pragma once

#define SI446X_CMD_POWER_UP 0x02
#define SI446X_CMD_NOP 0x00
#define SI446X_CMD_PART_INFO 0x01
#define SI446X_CMD_FUNC_INFO 0x10
#define SI446X_CMD_SET_PROPERTY 0x11
#define SI446X_CMD_GET_PROPERTY 0x12
#define SI446X_CMD_GPIO_PIN_CFG 0x13
#define SI446X_CMD_FIFO_INFO 0x15
#define SI446X_CMD_GET_INT_STATUS 0x20
#define SI446X_CMD_REQUEST_DEVICE_STATE 0x33
#define SI446X_CMD_CHANGE_STATE 0x34
#define SI446X_CMD_READ_CMD_BUFF 0x44
#define SI446X_CMD_READ_FRR_A 0x50
#define SI446X_CMD_READ_FRR_B 0x51
#define SI446X_CMD_READ_FRR_C 0x53
#define SI446X_CMD_READ_FRR_D 0x57
#define SI446X_CMD_IRCAL 0x17
#define SI446X_CMD_IRCAL_MANUAL 0x1a
#define SI446X_CMD_START_TX 0x31
#define SI446X_CMD_TX_HOP 0x37
#define SI446X_CMD_WRITE_TX_FIFO 0x66
#define SI446X_CMD_PACKET_INFO 0x16
#define SI446X_CMD_GET_MODEM_STATUS 0x22
#define SI446X_CMD_START_RX 0x32
#define SI446X_CMD_RX_HOP 0x36
#define SI446X_CMD_READ_RX_FIFO 0x77
#define SI446X_CMD_GET_ADC_READING 0x14
#define SI446X_CMD_GET_PH_STATUS 0x21
#define SI446X_CMD_GET_CHIP_STATUS 0x23

#define SI446X_INT_CTL_CHIP_LOW_BATT_EN 1
#define SI446X_INT_CTL_CHIP_WUT_EN 0

typedef enum
{
	SI446X_ADC_CONV_TEMP = 16,
	SI446X_ADC_CONV_BATT = 8,
	SI446X_ADC_CONV_GPIO = 4
} si446x_adc_conv_t;

typedef enum
{
	SI446X_ADC_RANGE_0P8 = 0,
	SI446X_ADC_RANGE_1P6 = 4,
	SI446X_ADC_RANGE_3P2 = 5,
	SI446X_ADC_RANGE_2P4 = 8,
	SI446X_ADC_RANGE_3P6 = 9
} si446x_adc_range_t;

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

#define SI446X_FIFO_CLEAR_RX 0x02
#define SI446X_FIFO_CLEAR_TX 0x01

#define GLOBAL_PROP(prop) ((SI446X_PROP_GROUP_GLOBAL << 8) | prop)
#define INT_PROP(prop) ((SI446X_PROP_GROUP_INT << 8) | prop)
#define PKT_PROP(prop) ((SI446X_PROP_GROUP_PKT << 8) | prop)
#define PA_PROP(prop) ((SI446X_PROP_GROUP_PA << 8) | prop)
#define MATCH_PROP(prop) ((SI446X_PROP_GROUP_MATCH << 8) | prop)

#define SI446X_GLOBAL_CONFIG GLOBAL_PROP(0x03)
#define SI446X_FIFO_MODE_HALF_DUPLEX 0x10

#define SI446X_GLOBAL_CLK_CFG GLOBAL_PROP(0x01)
#define SI446X_DIVIDED_CLK_DIS 0x00
#define SI446X_DIVIDED_CLK_EN 0x40
#define SI446X_DIVIDED_CLK_SEL_DIV_1 0 << 5
#define SI446X_DIVIDED_CLK_SEL_DIV_2 1 << 5
#define SI446X_DIVIDED_CLK_SEL_DIV_3 2 << 5
#define SI446X_DIVIDED_CLK_SEL_DIV_7_5 3 << 5
#define SI446X_DIVIDED_CLK_SEL_DIV_10 4 << 5
#define SI446X_DIVIDED_CLK_SEL_DIV_15 5 << 5
#define SI446X_DIVIDED_CLK_SEL_DIV_30 6 << 5
#define SI446X_DIVIDED_CLK_32K_SEL_OFF 0x00
#define SI446X_DIVIDED_CLK_32K_SEL_RC 0x01
#define SI446X_DIVIDED_CLK_32K_SEL_XTAL 0x02

#define SI446X_GLOBAL_LOW_BATT_THRESH GLOBAL_PROP(0x02)
#define SI446X_GLOBAL_WUT_CONFIG GLOBAL_PROP(0x04)
#define SI446X_GLOBAL_WUT_M GLOBAL_PROP(0x05)
#define SI446X_GLOBAL_WUT_R GLOBAL_PROP(0x07)
#define SI446X_GLOBAL_WUT_LDC GLOBAL_PROP(0x08)
#define SI446X_WUT_SLEEP 5
#define SI446X_LDC_MAX_PERIODS_FOREVER 0 << 6
#define SI446X_LDC_MAX_PERIODS_TWO 1 << 6
#define SI446X_LDC_MAX_PERIODS_FOUR 2 << 6
#define SI446X_LDC_MAX_PERIODS_EIGHT 3 << 6
#define SI446X_GLOBAL_WUT_CONFIG_WUT_LDC_EN_RX 1 << 6
#define SI446X_GLOBAL_WUT_CONFIG_WUT_EN 1
#define SI446X_GLOBAL_WUT_CONFIG_WUT_LBD_EN 2

#define SI446X_INT_CTL_ENABLE INT_PROP(0x00)
#define SI446X_INT_CTL_PH_ENABLE INT_PROP(0x01)
#define SI446X_INT_CTL_MODEM_ENABLE INT_PROP(0x02)
#define SI446X_INT_CTL_CHIP_ENABLE INT_PROP(0x03)

//INT_CTL_MODEM_ENABLE
const byte SYNC_DETECT_EN = 0;

//INT_CTL_PH_ENABLE
const byte PACKET_SENT_EN = 5;
const byte PACKET_RX_EN = 4;
const byte CRC_ERROR_EN = 3;
const byte TX_FIFO_ALMOST_EMPTY_EN = 1;
const byte RX_FIFO_ALMOST_FULL_EN = 0;

//GET_INT_STATUS
//	PH_CLR_PEND
const byte SI446X_FILTER_MATCH_PEND = 7;
const byte SI446X_FILTER_MISS_PEND = 6;
const byte SI446X_PACKET_SENT_PEND = 5;
const byte SI446X_PACKET_RX_PEND = 4;
const byte SI446X_CRC_ERROR_PEND = 3;
const byte TX_FIFO_ALMOST_EMPTY_PEND = 1;
const byte SI446X_RX_FIFO_ALMOST_FULL_PEND = 0;
//	MODEM_CLR_PEND
const byte SI446X_INVALID_SYNC_PEND = 5;
const byte SI446X_SYNC_DETECT_PEND = 0;
//	CHIP_CLR_PEND
const byte SI446X_LOW_BATT_PEND = 1;
const byte SI446X_WUT_PEND = 0;

#define SI446X_MATCH_VALUE_1 MATCH_PROP(0x00)
#define SI446X_MATCH_EN 0x40

#define SI446X_PA_PWR_LVL PA_PROP(0x01)

#define SI446X_PKT_FIELD_1_LENGTH PKT_PROP(0x0D)
#define SI446X_PKT_FIELD_2_LENGTH PKT_PROP(0x11)
#define SI446X_PKT_FIELD_2_LENGTH_LOW PKT_PROP(0x12)

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