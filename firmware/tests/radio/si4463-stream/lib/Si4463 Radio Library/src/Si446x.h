/*
 * Project: Si4463 Radio Library for AVR and Arduino
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2017 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/si4463-radio-library-avr-arduino/
 */

#pragma once

#include <Arduino.h>
#include "Si446x_config.h"
#include "Si446x_defs.h"

class Si446x : public Stream
{
public:
	Si446x();
	void setPins(int cs, int irq, int sdn);
	void begin(byte channel);

	// from Print
	virtual size_t write(uint8_t byte);
	virtual size_t write(const uint8_t *buffer, size_t size);

	// from Stream
	virtual int available();
	virtual int read();
	virtual int peek();
	virtual void flush();
	bool beginPacket();

	void getInfo(si446x_info_t *info);
	short getLatchedRSSI(void);
	short getRSSI(void);
	void setTxPower(byte pwr);
	void setupCallback(word callbacks, byte state);
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
	void onReceive(void (*callback)(byte));
	void onReceiveBegin(void (*callback)(short));
	void onReceiveInvalid(void (*callback)(short));
	void onSent(void (*callback)(void));
	void onBatteryLow(void (*callback)(void));
	void onWakingUp(void (*callback)(void));
	void onTxDone(void(*callback)());
	byte endPacket(si446x_state_t onTxFinish);
	void receive();
	void read(byte *buff, byte len);

private:
	byte interrupt_off(void);
	byte interrupt_on(void);
	static void onIrqFalling(void);
	void handleIrqFall(void);
	byte getResponse(byte *buff, byte len);
	byte waitForResponse(byte *out, byte outLen, byte useTimeout);
	void irq_off(void);
	void irq_on(void);
	void doAPI(byte *data, byte len, byte *out, byte outLen);
	void setProperties(word prop, byte *values, byte len);
	void setProperty(word prop, byte value);
	void getProperties(word prop, byte *values, byte len);
	byte getProperty(word prop);
	word getADC(byte adc_en, byte adc_cfg, byte part);
	byte getFRR(byte reg);
	si446x_state_t getState(void);
	void setState(si446x_state_t newState);
	void clearFIFO(void);
	void getFifoInfo(byte& RX_FIFO_COUNT, byte &TX_FIFO_SPACE);
	void interrupt(byte *buff);
	void interrupt2(byte *buff, byte clearPH, byte clearMODEM, byte clearCHIP);
	void resetDevice(void);
	void applyStartupConfig(void);
	void (*_onReceive)(byte) = nullptr;
	void (*_onReceiveBegin)(short) = nullptr;
	void (*_onReceiveInvalid)(short) = nullptr;
	void (*_onBatteryLow)(void) = nullptr;
	void (*_onWakingUp)(void) = nullptr;
	void (*_onTxDone)() = nullptr;
	byte _cs = 0;
	byte _irq = 0;
	byte _sdn = 0;
	byte _channel;
};