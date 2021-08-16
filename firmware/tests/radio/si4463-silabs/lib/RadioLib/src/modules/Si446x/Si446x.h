#pragma once

#include "../../TypeDef.h"

#include "../../Module.h"
#include "../../protocols/PhysicalLayer/PhysicalLayer.h"
#include "si446x_cmd.h"

// Si443x physical layer properties
#define SI4463_FREQUENCY_STEP_SIZE 28.6
#define SI4463_MAX_PACKET_LENGTH 64
#define RADIO_CTS_TIMEOUT 10000
#define SI446X_MAX_TX_POWER 127

#define IRQ_PACKET 0
#define IRQ_MODEM 1
#define IRQ_CHIP 2

enum
{
	SI446X_SUCCESS,
	SI446X_NO_PATCH,
	SI446X_CTS_TIMEOUT,
	SI446X_PATCH_FAIL,
	SI446X_COMMAND_ERROR
};

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

#define SI446X_FIFO_CLEAR_RX 0x02
#define SI446X_FIFO_CLEAR_TX 0x01

class Si446x : public PhysicalLayer
{
public:
	// introduce PhysicalLayer overloads
	using PhysicalLayer::readData;
	using PhysicalLayer::receive;
	using PhysicalLayer::startTransmit;
	using PhysicalLayer::transmit;
	Si446x(Module *mod);
	int16_t begin();
	void reset();
	int16_t transmit(uint8_t *data, size_t len, uint8_t addr = 0);
	int16_t transmitDirect(uint32_t frf = 0) { return -1; };
	int16_t receive(uint8_t *data, size_t len);
	int16_t receiveDirect() { return -1; };
	int16_t standby() { return -1; };
	int16_t setFrequencyDeviation(float freqDev) { return -1; };
	int16_t setDataShaping(uint8_t sh) { return -1; };
	int16_t setEncoding(uint8_t encoding) { return -1; };
	size_t getPacketLength(bool update = true);
	int16_t startTransmit(uint8_t *data, size_t len, uint8_t addr = 0);
	int16_t startReceive();
	int16_t readData(uint8_t *data, size_t len);
	float getLatchedRssi();
	uint8_t random() { return -1; };
	int16_t sleep();
	void setTxPower(uint8_t pwr);
	void setChannel(byte channel);
	// void setIrqAction(void (*func)(void));
	// void clearIrqAction();
	int16_t getChipVersion();
	bool pollIRQLine();

protected:
	Module *_mod;

private:
	si446x_state_t getState();
	volatile uint8_t enabledInterrupts[3];
	byte _channel;
	size_t _packetLength = 0;
	float _br = 2.400;
	uint32_t start_rx_timeout;

	//API-lib
	si446x_cmd_reply_union Si446xCmd;
	byte Pro2Cmd[16];
	void si446x_reset(void);
	bool si446x_part_info(void);
	bool si446x_current_rssi(float* rssi);
	float rssi_regval_to_dBm(byte regVal);
	void clearIRQFlags();
	bool si446x_get_int_status(byte PH_CLR_PEND, byte MODEM_CLR_PEND, byte CHIP_CLR_PEND);
	byte si446x_configuration_init(const byte *pSetPropCmd);
	void si446x_set_property(byte GROUP, byte NUM_PROPS, byte START_PROP, void *values, byte len);
	byte si446x_get_fastResponse(byte reg);
	void si446x_change_state(byte NEXT_STATE1);
	void si446x_fifo_info_fast_reset(byte FIFO);
	void si446x_read_rx_fifo(byte numBytes, byte *pRxData);
	void si446x_write_tx_fifo(byte numBytes, byte *pTxData);
	void si446x_start_rx(byte CHANNEL, byte CONDITION, word RX_LEN, byte NEXT_STATE1, byte NEXT_STATE2, byte NEXT_STATE3);
	void si446x_start_tx(byte CHANNEL, byte CONDITION, word TX_LEN);

	//Radio_cmd
	byte radioCmd[16u];
	byte radio_comm_GetResp(byte byteCount, byte *pData);
	void radio_comm_SendCmd(byte byteCount, byte *pData);
	void radio_comm_ReadData(byte cmd, bool pollCts, byte byteCount, byte *pData);
	void radio_comm_WriteData(byte cmd, bool pollCts, byte byteCount, byte *pData);
	byte radio_comm_PollCTS(void);
	byte radio_comm_SendCmdGetResp(byte cmdByteCount, byte *pCmdData, byte respByteCount, byte *pRespData);
	void radio_comm_ClearCTS(void);
	bool ctsWentHigh = 0;
};
