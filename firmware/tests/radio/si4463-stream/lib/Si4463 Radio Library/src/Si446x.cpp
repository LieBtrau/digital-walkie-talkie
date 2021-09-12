/*
 * Project: Si4463 Radio Library for AVR and Arduino
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2017 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/si4463-radio-library-avr-arduino/
 */

#include <Arduino.h>
#include "SPI.h"
#include "Si446x.h"
#include "Si446x_config.h"
#include "Si446x_defs.h"
#include "radio_config.h"

#if (ESP8266 || ESP32)
#define ISR_PREFIX ICACHE_RAM_ATTR
#else
#define ISR_PREFIX
#endif

#define IDLE_STATE SI446X_IDLE_MODE

// When FIFOs are combined it becomes a 129 byte FiFO
// The first byte is used for length, then the remaining 128 bytes for the packet data
#define MAX_PACKET_LEN SI446X_MAX_PACKET_LEN

static volatile struct
{
	byte INT_CTL_PH_ENABLE;
	byte INT_CTL_MODEM_ENABLE;
	byte INT_CTL_CHIP_ENABLE;
} cached_Int_Enable;

#define rssi_dBm(val) ((val / 2) - 134)

static Si446x *pSi446x = nullptr;

static const byte config[] PROGMEM = RADIO_CONFIGURATION_DATA_ARRAY;

// It's not possible to get the current interrupt enabled state in Arduino (SREG only works for AVR based Arduinos, and no way of getting attachInterrupt() status), so we use a counter thing instead
static volatile byte isrState_local;
static volatile byte isrState;
static volatile byte isrBusy; // Don't mess with global interrupts if we're inside an ISR

Si446x::Si446x()
{
	pSi446x = this;
}

void Si446x::onReceive(void (*callback)(byte))
{
	_onReceive = callback;
}

void Si446x::onReceiveBegin(void (*callback)(short))
{
	_onReceiveBegin = callback;
}

void Si446x::onReceiveInvalid(void (*callback)(short))
{
	_onReceiveInvalid = callback;
}

void Si446x::onBatteryLow(void (*callback)(void))
{
	_onBatteryLow = callback;
}

void Si446x::onWakingUp(void (*callback)(void))
{
	_onWakingUp = callback;
}

void Si446x::onTxDone(void (*callback)(void))
{
	_onTxDone = callback;
}

// Configure a bunch of properties (up to 12 properties in one go)
void Si446x::setProperties(word prop, byte *values, byte len)
{
	// len must not be greater than 12
	byte data[16] = {
		SI446X_CMD_SET_PROPERTY,
		(byte)(prop >> 8),
		len,
		(byte)prop};

	// Copy values into data, starting at index 4
	memcpy(data + 4, values, len);

	doAPI(data, len + 4, NULL, 0);
}

// Set a single property
inline void Si446x::setProperty(word prop, byte value)
{
	setProperties(prop, &value, 1);
}

// Read a bunch of properties
void Si446x::getProperties(word prop, byte *values, byte len)
{
	byte data[] =
		{
			SI446X_CMD_GET_PROPERTY,	//CMD
			highByte(prop),				//GROUP
			len,						//NUM_PROPS	
			lowByte(prop)};				//START_PROP

	doAPI(data, sizeof(data), values, len);
}

// Read a single property
inline byte Si446x::getProperty(word prop)
{
	byte val;
	getProperties(prop, &val, 1);
	return val;
}

// Do an ADC conversion
word Si446x::getADC(byte adc_en, byte adc_cfg, byte part)
{
	byte data[6] = {
		SI446X_CMD_GET_ADC_READING,
		adc_en,
		adc_cfg};
	doAPI(data, 3, data, 6);
	return (data[part] << 8 | data[part + 1]);
}

// Get the patched RSSI from the beginning of the packet
short Si446x::getLatchedRSSI(void)
{
	byte frr = getFRR(SI446X_CMD_READ_FRR_A);
	short rssi = rssi_dBm(frr);
	return rssi;
}

/**
* @brief Get the radio status
*
* @see ::si446x_state_t
* @return The current radio status
*/
si446x_state_t Si446x::getState(void)
{
	byte state = getFRR(SI446X_CMD_READ_FRR_B);
	switch (state)
	{
	case SI446X_STATE_TX_TUNE:
		return SI446X_STATE_TX;
	case SI446X_STATE_RX_TUNE:
		return SI446X_STATE_RX;
	case SI446X_STATE_READY2:
		return SI446X_STATE_READY;
	default:
		return (si446x_state_t)state;
	}
}

// Set new state
void Si446x::setState(si446x_state_t newState)
{
	byte data[] = {
		SI446X_CMD_CHANGE_STATE,
		newState};
	doAPI(data, sizeof(data), NULL, 0);
}

// Clear RX and TX FIFOs
void Si446x::clearFIFO(void)
{
	// 'static const' saves 20 bytes of flash here, but uses 2 bytes of RAM
	static const byte clearFifo[] = {
		SI446X_CMD_FIFO_INFO,
		SI446X_FIFO_CLEAR_RX | SI446X_FIFO_CLEAR_TX};
	doAPI((byte *)clearFifo, sizeof(clearFifo), NULL, 0);
}

// Read and clear pending interrupts
// Buff should either be NULL (just clear interrupts) or a buffer of at least 8 bytes for storing statuses
void Si446x::interrupt(byte *buff)
{
	byte data = SI446X_CMD_GET_INT_STATUS;
	doAPI(&data, sizeof(data), buff, 8);
}

// Similar to interrupt() but with the option of not clearing certain interrupt flags
void Si446x::interrupt2(byte *buff, byte clearPH, byte clearMODEM, byte clearCHIP)
{
	byte data[] = {
		SI446X_CMD_GET_INT_STATUS,
		clearPH,
		clearMODEM,
		clearCHIP};
	doAPI(data, sizeof(data), buff, 8);
}

// Reset the RF chip
void Si446x::resetDevice(void)
{
	digitalWrite(_sdn, HIGH);
	delay(50);
	digitalWrite(_sdn, LOW);
	delay(50);
}

// Apply the radio configuration
void Si446x::applyStartupConfig(void)
{
	byte buff[17];
	for (word i = 0; i < sizeof(config); i++)
	{
		memcpy_P(buff, &config[i], sizeof(buff));
		doAPI(&buff[1], buff[0], NULL, 0);
		i += buff[0];
	}
}

void Si446x::setPins(int cs, int irq, int sdn)
{
	_cs = cs;
	_irq = irq;
	_sdn = sdn;
}

/**
* @brief Initialise, must be called before anything else!
*
* @return (none)
*/
void Si446x::begin(byte channel)
{
	if (_cs == 0 || _irq == 0 || _sdn == 0)
	{
		return;
	}
	_channel = channel;
	digitalWrite(_cs, HIGH);
	pinMode(_cs, OUTPUT);
	pinMode(_sdn, OUTPUT);
	pinMode(_irq, INPUT_PULLUP);
	SPI.begin();
	resetDevice();
	applyStartupConfig();
	interrupt(nullptr);
	sleep();

	//Enable individual interrupt sources within the Packet Handler Interrupt Group to generate a HW interrupt on the NIRQ output pin.
	bitSet(cached_Int_Enable.INT_CTL_PH_ENABLE, PACKET_SENT_EN);
	bitSet(cached_Int_Enable.INT_CTL_PH_ENABLE, PACKET_RX_EN);
	bitSet(cached_Int_Enable.INT_CTL_PH_ENABLE, CRC_ERROR_EN);
	irq_off();
	setProperties(SI446X_INT_CTL_PH_ENABLE, (byte *)&cached_Int_Enable.INT_CTL_PH_ENABLE, 1);
	irq_on();

	if (isrState_local > 0)
		isrState_local--;
	if (isrState_local == 0)
		attachInterrupt(digitalPinToInterrupt(_irq), onIrqFalling, FALLING);
}

/**
* @brief Get chip info, see ::si446x_info_t
*
* @see ::si446x_info_t
* @param [info] Pointer to allocated ::si446x_info_t struct to place data into
* @return (none)
*/
void Si446x::getInfo(si446x_info_t *info)
{
	byte data[8] =
		{
			SI446X_CMD_PART_INFO};
	doAPI(data, 1, data, 8);

	info->chipRev = data[0];
	info->part = (data[1] << 8) | data[2];
	info->partBuild = data[3];
	info->id = (data[4] << 8) | data[5];
	info->customer = data[6];
	info->romId = data[7];

	data[0] = SI446X_CMD_FUNC_INFO;
	doAPI(data, 1, data, 6);

	info->revExternal = data[0];
	info->revBranch = data[1];
	info->revInternal = data[2];
	info->patch = (data[3] << 8) | data[4];
	info->func = data[5];
}

/**
* @brief Get the current RSSI, the chip needs to be in receive mode for this to work
*
* @return The current RSSI in dBm (usually between -130 and 0)
*/
short Si446x::getRSSI()
{
	byte data[3] = {
		SI446X_CMD_GET_MODEM_STATUS,
		0xFF};
	doAPI(data, 2, data, 3);
	short rssi = rssi_dBm(data[2]);
	return rssi;
}

/**
* @brief Set the transmit power. The output power does not follow the \p pwr value, see the Si446x datasheet for a pretty graph
*
* 0 = -32dBm (<1uW)\n
* 7 = 0dBm (1mW)\n
* 12 = 5dBm (3.2mW)\n
* 22 = 10dBm (10mW)\n
* 40 = 15dBm (32mW)\n
* 100 = 20dBm (100mW)
*
* @param [pwr] A value from 0 to 127
* @return (none)
*/
void Si446x::setTxPower(byte pwr)
{
	setProperty(SI446X_PA_PWR_LVL, pwr);
}

/**
* @brief Set the low battery voltage alarm
*
* The ::SI446X_CB_LOWBATT() callback will be ran when the supply voltage drops below this value. The WUT must be configured with ::Si446x_setupWUT() to enable periodically checking the battery level.
*
* @param [voltage] The low battery threshold in millivolts (1050 - 3050).
* @return (none)
*/
void Si446x::setLowBatt(word voltage)
{
	// voltage should be between 1500 and 3050
	byte batt = (voltage / 50) - 30; //((voltage * 2) - 3000) / 100;
	setProperty(SI446X_GLOBAL_LOW_BATT_THRESH, batt);
}

/**
* @brief Configure the wake up timer
* 
* This function will also reset the timer.\n
*\n
* The Wake Up Timer (WUT) can be used to periodically run a number of features:\n
* ::SI446X_WUT_RUN Simply wake up the microcontroller when the WUT expires and run the ::SI446X_CB_WUT() callback.\n
* ::SI446X_WUT_BATT Check battery voltage - If the battery voltage is below the threshold set by ::Si446x_setLowBatt() then wake up the microcontroller and run the ::SI446X_CB_LOWBATT() callback.\n
* ::SI446X_WUT_RX Enter receive mode for a length of time determinded by the ldc and r parameters (NOT SUPPORTED YET! dont use this option)\n
*\n
* For more info see the GLOBAL_WUT_M, GLOBAL_WUT_R and GLOBAL_WUT_LDC properties in the Si446x API docs.\n
*
* @note When first turning on the WUT this function will take around 300us to complete
* @param [r] Exponent value for WUT and LDC (Maximum valus is 20)
* @param [m] Mantissia value for WUT
* @param [ldc] Mantissia value for LDC (NOT SUPPORTED YET, just pass 0 for now)
* @param [config] Which WUT features to enable ::SI446X_WUT_RUN ::SI446X_WUT_BATT ::SI446X_WUT_RX These can be bitwise OR'ed together to enable multiple features.
* @return (none)
*/
void Si446x::setupWUT(byte r, word m, byte ldc, byte config)
{
	// Maximum value of r is 20

	// The API docs say that if r or m are 0, then they will have the same effect as if they were 1, but this doesn't seem to be the case?

	// Check valid config
	// TODO needed?
	if (!(config & (SI446X_WUT_RUN | SI446X_WUT_BATT | SI446X_WUT_RX)))
		return;

	irq_off();
	// Disable WUT
	setProperty(SI446X_GLOBAL_WUT_CONFIG, 0);

	byte doRun = !!(config & SI446X_WUT_RUN);
	byte doBatt = !!(config & SI446X_WUT_BATT);
	byte doRx = (config & SI446X_WUT_RX);

	// Setup WUT interrupts
	byte intChip = 0; //getProperty(SI446X_INT_CTL_CHIP_ENABLE); // No other CHIP interrupts are enabled so dont bother reading the current state
	//intChip &= ~((1<<SI446X_INT_CTL_CHIP_LOW_BATT_EN)|(1<<SI446X_INT_CTL_CHIP_WUT_EN));
	intChip |= doBatt << SI446X_INT_CTL_CHIP_LOW_BATT_EN;
	intChip |= doRun << SI446X_INT_CTL_CHIP_WUT_EN;
	cached_Int_Enable.INT_CTL_CHIP_ENABLE = intChip;
	setProperty(SI446X_INT_CTL_CHIP_ENABLE, intChip);

	// Set WUT clock source to internal 32KHz RC
	if (getProperty(SI446X_GLOBAL_CLK_CFG) != SI446X_DIVIDED_CLK_32K_SEL_RC)
	{
		setProperty(SI446X_GLOBAL_CLK_CFG, SI446X_DIVIDED_CLK_32K_SEL_RC);
		delayMicroseconds(300); // Need to wait 300us for clock source to stabilize, see GLOBAL_WUT_CONFIG:WUT_EN info
	}

	// Setup WUT
	byte properties[5];
	properties[0] = doRx ? SI446X_GLOBAL_WUT_CONFIG_WUT_LDC_EN_RX : 0;
	properties[0] |= doBatt << SI446X_GLOBAL_WUT_CONFIG_WUT_LBD_EN;
	properties[0] |= (1 << SI446X_GLOBAL_WUT_CONFIG_WUT_EN);
	properties[1] = m >> 8;
	properties[2] = m;
	properties[3] = r | SI446X_LDC_MAX_PERIODS_TWO | (1 << SI446X_WUT_SLEEP);
	properties[4] = ldc;
	setProperties(SI446X_GLOBAL_WUT_CONFIG, properties, sizeof(properties));
	irq_on();
}

/**
* @brief Disable the wake up timer
*
* @return (none)
*/
void Si446x::disableWUT()
{
	irq_off();
	setProperty(SI446X_GLOBAL_WUT_CONFIG, 0);
	setProperty(SI446X_GLOBAL_CLK_CFG, 0);
	irq_on();
}

/**
* @brief Enable or disable callbacks. This is mainly to configure what events should wake the microcontroller up.
*
* @param [callbacks] The callbacks to configure (multiple callbacks should be bitewise OR'd together)
* @param [state] Enable or disable the callbacks passed in \p callbacks parameter (1 = Enable, 0 = Disable)
* @return (none)
*/
void Si446x::setupCallback(word callbacks, byte state)
{
	irq_off();
	byte data[2];
	getProperties(SI446X_INT_CTL_PH_ENABLE, data, sizeof(data));

	if (state)
	{
		data[0] |= highByte(callbacks);
		data[1] |= lowByte(callbacks);
	}
	else
	{
		data[0] &= ~highByte(callbacks);
		data[1] &= ~lowByte(callbacks);
	}

	// TODO
	// make sure RXCOMPELTE, RXINVALID and RXBEGIN? are always enabled
	cached_Int_Enable.INT_CTL_PH_ENABLE = data[0];
	cached_Int_Enable.INT_CTL_MODEM_ENABLE = data[1];
	setProperties(SI446X_INT_CTL_PH_ENABLE, data, sizeof(data));
	irq_on();
}

/**
* @brief Enter sleep mode
*
* If WUT is enabled then the radio will keep the internal 32KHz RC enabled with a current consumption of 740nA, otherwise the current consumption will be 40nA without WUT.
* Sleep will fail if the radio is currently transmitting.
*
* @note Any SPI communications with the radio will wake the radio into ::SI446X_STATE_SPI_ACTIVE mode. ::Si446x_sleep() will need to called again to put it back into sleep mode.
*
* @return 0 on failure (busy transmitting something), 1 on success
*/
byte Si446x::sleep()
{
	if (getState() == SI446X_STATE_TX)
		return 0;
	setState(SI446X_STATE_SLEEP);
	return 1;
}

/**
* @brief Enter receive mode
*
* Entering RX mode will abort any transmissions happening at the time
*
* @param [channel] Channel to listen to (0 - 255)
* @return (none)
*/
void Si446x::receive()
{
	irq_off();
	setState(IDLE_STATE);
	clearFIFO();
	//fix_invalidSync_irq(0);
	//Si446x_setupCallback(SI446X_CBS_INVALIDSYNC, 0);
	//setProperty(SI446X_PKT_FIELD_2_LENGTH_LOW, MAX_PACKET_LEN); // TODO ?
	interrupt2(NULL, 0, 0, 0xFF); // TODO needed?

	// TODO RX timeout to sleep if WUT LDC enabled

	byte data[] = {
		SI446X_CMD_START_RX,
		_channel,
		0,
		0,
		SI446X_FIXED_LENGTH,
		SI446X_STATE_NOCHANGE, // RX Timeout
		IDLE_STATE,			   // RX Valid
		SI446X_STATE_SLEEP	   // IDLE_STATE // RX Invalid (using SI446X_STATE_SLEEP for the INVALID_SYNC fix)
	};
	doAPI(data, sizeof(data), NULL, 0);
	irq_on();
}

/**
* @brief Read pin ADC value
*
* @param [pin] The GPIO pin number (0 - 3)
* @return ADC value (0 - 2048, where 2048 is 3.6V)
*/
word Si446x::adc_gpio(byte pin)
{
	word result = getADC(SI446X_ADC_CONV_GPIO | pin, (SI446X_ADC_SPEED << 4) | SI446X_ADC_RANGE_3P6, 0);
	return result;
}

/**
* @brief Read supply voltage
*
* @return Supply voltage in millivolts
*/
word Si446x::adc_battery()
{
	word result = getADC(SI446X_ADC_CONV_BATT, (SI446X_ADC_SPEED << 4), 2);
	result = ((uint32_t)result * 75) / 32; // result * 2.34375;
	return result;
}

/**
* @brief Read temperature
*
* @return Temperature in C
*/
float Si446x::adc_temperature()
{
	float result = getADC(SI446X_ADC_CONV_TEMP, (SI446X_ADC_SPEED << 4), 4);
	result = (899 / 4096.0) * result - 293;
	return result;
}

/**
* @brief Configure GPIO/NIRQ/SDO pin
*
* @note NIRQ and SDO pins should not be changed, unless you really know what you're doing. 2 of the GPIO pins (usually 0 and 1) are also usually used for the RX/TX RF switch and should also be left alone.
*
* @param [pin] The pin, this can only take a single pin (don't use bitwise OR), see ::si446x_gpio_t
* @param [value] The new pin mode, this can be bitwise OR'd with the ::SI446X_PIN_PULL_EN option, see ::si446x_gpio_mode_t ::si446x_nirq_mode_t ::si446x_sdo_mode_t
* @return (none)
*/
void Si446x::writeGPIO(si446x_gpio_t pin, byte value)
{
	byte data[] = {
		SI446X_CMD_GPIO_PIN_CFG,
		SI446X_GPIO_MODE_DONOTHING,
		SI446X_GPIO_MODE_DONOTHING,
		SI446X_GPIO_MODE_DONOTHING,
		SI446X_GPIO_MODE_DONOTHING,
		SI446X_NIRQ_MODE_DONOTHING,
		SI446X_SDO_MODE_DONOTHING,
		SI446X_GPIO_DRV_HIGH};
	data[pin + 1] = value;
	doAPI(data, sizeof(data), NULL, 0);
}

/**
* @brief Read GPIO pin states
*
* @return The pin states. Use ::si446x_gpio_t to mask the value to get the state for the desired pin.
*/
byte Si446x::readGPIO()
{
	byte data[4] = {
		SI446X_CMD_GPIO_PIN_CFG};
	doAPI(data, 1, data, sizeof(data));
	byte states = data[0] >> 7 | (data[1] & 0x80) >> 6 | (data[2] & 0x80) >> 5 | (data[3] & 0x80) >> 4;
	return states;
}

/**
* @brief Get all values of a property group
*
* @param [buff] Pointer to memory to place group values, if this is NULL then nothing will be dumped, just the group size is returned
* @param [group] The group to dump
* @return Size of the property group
*/
byte Si446x::dump(byte *buff, byte group)
{
	static const byte groupSizes[] PROGMEM = {
		SI446X_PROP_GROUP_GLOBAL, 0x0A,
		SI446X_PROP_GROUP_INT, 0x04,
		SI446X_PROP_GROUP_FRR, 0x04,
		SI446X_PROP_GROUP_PREAMBLE, 0x0E,
		SI446X_PROP_GROUP_SYNC, 0x06,
		SI446X_PROP_GROUP_PKT, 0x40,
		SI446X_PROP_GROUP_MODEM, 0x60,
		SI446X_PROP_GROUP_MODEM_CHFLT, 0x24,
		SI446X_PROP_GROUP_PA, 0x07,
		SI446X_PROP_GROUP_SYNTH, 0x08,
		SI446X_PROP_GROUP_MATCH, 0x0C,
		SI446X_PROP_GROUP_FREQ_CONTROL, 0x08,
		SI446X_PROP_GROUP_RX_HOP, 0x42,
		SI446X_PROP_GROUP_PTI, 0x04};

	byte length = 0;
	for (byte i = 0; i < sizeof(groupSizes); i += 2)
	{
		byte buff[2];
		memcpy_P(buff, &groupSizes[i], sizeof(buff));

		if (buff[0] == group)
		{
			length = buff[1];
			break;
		}
	}

	if (buff == NULL)
		return length;

	for (byte i = 0; i < length; i += 16)
	{
		byte count = length - i;
		if (count > 16)
			count = 16;
		getProperties((group << 8) | i, buff + i, count);
	}

	return length;
}

/**
* @brief If interrupts are disabled (::SI446X_INTERRUPTS in Si446x_config.h) then this function should be called as often as possible to process any events
*
* @return (none)
*/
ISR_PREFIX void Si446x::onIrqFalling()
{
	pSi446x->handleIrqFall();
}

void Si446x::handleIrqFall()
{
	isrBusy = 1;
	byte interrupts[8];
	interrupt(interrupts);
	// We could read the enabled interrupts properties instead of keep their states in RAM, but that would be much slower
	byte PH_PEND = interrupts[2] & cached_Int_Enable.INT_CTL_PH_ENABLE;
	byte MODEM_PEND = interrupts[4] & cached_Int_Enable.INT_CTL_MODEM_ENABLE;
	byte CHIP_PEND = interrupts[6] & cached_Int_Enable.INT_CTL_CHIP_ENABLE;

	// Packet sent
	if (bitRead(PH_PEND, SI446X_PACKET_SENT_PEND) && _onTxDone != nullptr)
	{
		_onTxDone();
	}

	// Valid PREAMBLE and SYNC, packet data now begins
	if (bitRead(MODEM_PEND, SI446X_SYNC_DETECT_PEND) && _onReceiveBegin != nullptr)
	{
		//fix_invalidSync_irq(1);
		//		Si446x_setupCallback(SI446X_CBS_INVALIDSYNC, 1); // Enable INVALID_SYNC when a new packet starts, sometimes a corrupted packet will mess the radio up
		_onReceiveBegin(getLatchedRSSI());
	}

	// Valid packet
	if (bitRead(PH_PEND, SI446X_PACKET_RX_PEND))
	{
#if !SI446X_FIXED_LENGTH
		byte len = 0;
		read(&len, 1);
#else
		byte len = SI446X_FIXED_LENGTH;
#endif
		if (_onReceive != nullptr)
		{
			_onReceive(len);
		}
	}

	// Corrupted packet
	// NOTE: This will still be called even if the address did not match, but the packet failed the CRC
	// This will not be called if the address missed, but the packet passed CRC
	if (bitRead(PH_PEND, SI446X_CRC_ERROR_PEND))
	{
#if IDLE_STATE == SI446X_STATE_READY
		if (getState() == SI446X_STATE_SPI_ACTIVE)
			setState(IDLE_STATE); // We're in sleep mode (acually, we're now in SPI active mode) after an invalid packet to fix the INVALID_SYNC issue
#endif
		if (_onReceiveInvalid != nullptr)
		{
			_onReceiveInvalid(getLatchedRSSI());
		}
	}

	if (bitRead(CHIP_PEND, SI446X_LOW_BATT_PEND) && _onBatteryLow != nullptr)
	{
		_onBatteryLow();
	}

	if (bitRead(CHIP_PEND, SI446X_WUT_PEND) && _onWakingUp != nullptr)
	{
		_onWakingUp();
	}

	isrBusy = 0;
}

bool Si446x::beginPacket()
{
	if (getState() == SI446X_STATE_TX) // Already transmitting
	{
		return false;
	}
	irq_off();
	setState(IDLE_STATE);
	clearFIFO();
	interrupt2(NULL, 0, 0, 0xFF);
	return true;
}

/**
* @brief Transmit a packet
*
* @param [packet] Pointer to packet data
* @param [len] Number of bytes to transmit, maximum of ::SI446X_MAX_PACKET_LEN If configured for fixed length packets then this parameter is ignored and the length is set by ::SI446X_FIXED_LENGTH in Si446x_config.h
* @param [channel] Channel to transmit data on (0 - 255)
* @param [onTxFinish] What state to enter when the packet has finished transmitting. Usually ::SI446X_STATE_SLEEP or ::SI446X_STATE_RX
* @return 0 on failure (already transmitting), 1 on success (has begun transmitting)
*/
byte Si446x::TX(byte *packet, byte len, si446x_state_t onTxFinish)
{
	// TODO what happens if len is 0?

#if SI446X_FIXED_LENGTH
	// Stop the unused parameter warning
	((void)(len));
#endif

	interrupt_off();
	// Load data to FIFO
	digitalWrite(_cs, LOW);
	SPI.transfer(SI446X_CMD_WRITE_TX_FIFO);
#if !SI446X_FIXED_LENGTH
	SPI.transfer(len);
	for (byte i = 0; i < len; i++)
		SPI.transfer(packet[i]);
#else
	for (byte i = 0; i < SI446X_FIXED_LENGTH; i++)
		SPI.transfer(packet[i]);
#endif
	digitalWrite(_cs, HIGH);
	interrupt_on();

#if !SI446X_FIXED_LENGTH
	// Set packet length
	setProperty(SI446X_PKT_FIELD_2_LENGTH_LOW, len);
#endif

	// Begin transmit
	byte data[] = {
		SI446X_CMD_START_TX,
		_channel,
		(byte)(onTxFinish << 4),
		0,
		SI446X_FIXED_LENGTH,
		0,
		0};
	doAPI(data, sizeof(data), NULL, 0);

#if !SI446X_FIXED_LENGTH
	// Reset packet length back to max for receive mode
	setProperty(SI446X_PKT_FIELD_2_LENGTH_LOW, MAX_PACKET_LEN);
#endif
	irq_on();
	return 1;
}

void Si446x::doAPI(byte *data, byte len, byte *out, byte outLen)
{
	irq_off();
	if (waitForResponse(NULL, 0, 1)) // Make sure it's ok to send a command
	{
		interrupt_off();
		digitalWrite(_cs, LOW);
		for (byte i = 0; i < len; i++)
		{
			SPI.transfer(data[i]); // (pgm_read_byte(&((byte*)data)[i]));
		}
		digitalWrite(_cs, HIGH);
		interrupt_on();
		if (data[0] == SI446X_CMD_IRCAL) // If we're doing an IRCAL then wait for its completion without a timeout since it can sometimes take a few seconds
		{
			waitForResponse(NULL, 0, 0);
		}
		else if (out != nullptr) // If we have an output buffer then read command response into it
		{
			waitForResponse(out, outLen, 1);
		}
	}
	irq_on();
}

// Keep trying to read the command buffer, with timeout of around 500ms
byte Si446x::waitForResponse(byte *out, byte outLen, byte useTimeout)
{
	// With F_CPU at 8MHz and SPI at 4MHz each check takes about 7us + 10us delay
	word timeout = 40000;
	while (!getResponse(out, outLen))
	{
		delayMicroseconds(10);
		if (useTimeout && !--timeout)
		{
			return 0;
		}
	}
	return 1;
}

// Read CTS and if its ok then read the command buffer
byte Si446x::getResponse(byte *buff, byte len)
{
	byte cts = 0;

	interrupt_off();
	digitalWrite(_cs, LOW);
	// Send command
	SPI.transfer(SI446X_CMD_READ_CMD_BUFF);
	// Get CTS value
	cts = (SPI.transfer(0xFF) == 0xFF);
	if (cts)
	{
		// Get response data
		for (byte i = 0; i < len; i++)
		{
			buff[i] = SPI.transfer(0xFF);
		}
	}
	digitalWrite(_cs, HIGH);
	interrupt_on();
	return cts;
}

/**
* @brief Read received data from FIFO
*
* @param [buff] Pointer to buffer to place data
* @param [len] Number of bytes to read, make sure not to read more bytes than what the FIFO has stored. The number of bytes that can be read is passed in the ::SI446X_CB_RXCOMPLETE() callback.
* @return (none)
*/
void Si446x::read(byte *buff, byte len)
{
	interrupt_off();
	digitalWrite(_cs, LOW);
	SPI.transfer(SI446X_CMD_READ_RX_FIFO);
	for (byte i = 0; i < len; i++)
	{
		buff[i] = SPI.transfer(0xFF);
	}
	digitalWrite(_cs, HIGH);
	interrupt_on();
}

// Read a fast response register
byte Si446x::getFRR(byte reg)
{
	byte frr = 0;
	interrupt_off();
	digitalWrite(_cs, LOW);
	SPI.transfer(reg);
	frr = SPI.transfer(0xFF);
	digitalWrite(_cs, HIGH);
	interrupt_on();
	return frr;
}

inline byte Si446x::interrupt_off(void)
{
	if (!isrBusy)
	{
		noInterrupts();
		isrState++;
	}
	return 1;
}

inline byte Si446x::interrupt_on(void)
{
	if (!isrBusy)
	{
		if (isrState > 0)
			isrState--;
		if (isrState == 0)
			interrupts();
	}
	return 0;
}

/**
* @brief When using interrupts use this to disable them for the Si446x
*
* Ideally you should wrap sensitive sections with ::SI446X_NO_INTERRUPT() instead, as it automatically deals with this function and ::Si446x_irq_on()
*
* @see ::Si446x_irq_on() and ::SI446X_NO_INTERRUPT()
* @return The previous interrupt status; 1 if interrupt was enabled, 0 if it was already disabled
*/
// TODO
// 2 types of interrupt blocks
// Local (SI446X_NO_INTERRUPT()): Disables the pin interrupt so the ISR does not run while normal code is busy in the Si446x code, however another interrupt can enter the code which would be bad.
// Global (SI446X_ATOMIC()): Disable all interrupts, don't use waitForResponse() inside here as it can take a while to complete. These blocks are to make sure no other interrupts use the SPI bus.

// When doing SPI comms with the radio or doing multiple commands we don't want the radio interrupt to mess it up.
void Si446x::irq_off()
{
	detachInterrupt(digitalPinToInterrupt(_irq));
	isrState_local++;
}

/**
* @brief When using interrupts use this to re-enable them for the Si446x
*
* Ideally you should wrap sensitive sections with ::SI446X_NO_INTERRUPT() instead, as it automatically deals with this function and ::Si446x_irq_off()
*
* @see ::Si446x_irq_off() and ::SI446X_NO_INTERRUPT()
* @param [origVal] The original interrupt status returned from ::Si446x_irq_off()
* @return (none)
*/
void Si446x::irq_on()
{
	if (isrState_local > 0)
		isrState_local--;
	if (isrState_local == 0)
		attachInterrupt(digitalPinToInterrupt(_irq), Si446x::onIrqFalling, FALLING);
}
