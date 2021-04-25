#include "Si446x.h"

/*!
 * This functions is used to reset the si446x radio by applying shutdown and 
 * releasing it.  After this function @ref si446x_boot should be called.  You
 * can check if POR has completed by waiting 4 ms or by polling GPIO 0, 2, or 3.
 * When these GPIOs are high, it is safe to call @ref si446x_boot.
 */
void Si446x::si446x_reset(void)
{
	Module::digitalWrite(_mod->getRst(), HIGH);
	Module::pinMode(_mod->getRst(), OUTPUT);
	Module::delay(10);
	Module::digitalWrite(_mod->getRst(), LOW);
	Module::delay(100);
	radio_comm_ClearCTS();
}

/*! This function sends the PART_INFO command to the radio and receives the answer
 *  into @Si446xCmd union.
 */
bool Si446x::si446x_part_info(void)
{
	Pro2Cmd[0] = SI446X_CMD_ID_PART_INFO;

	byte ctsVal = radio_comm_SendCmdGetResp(SI446X_CMD_ARG_COUNT_PART_INFO,
											Pro2Cmd,
											SI446X_CMD_REPLY_COUNT_PART_INFO,
											Pro2Cmd);

	Si446xCmd.PART_INFO.CHIPREV = Pro2Cmd[0];
	Si446xCmd.PART_INFO.PART = ((U16)Pro2Cmd[1] << 8) & 0xFF00;
	Si446xCmd.PART_INFO.PART |= (U16)Pro2Cmd[2] & 0x00FF;
	Si446xCmd.PART_INFO.PBUILD = Pro2Cmd[3];
	Si446xCmd.PART_INFO.ID = ((U16)Pro2Cmd[4] << 8) & 0xFF00;
	Si446xCmd.PART_INFO.ID |= (U16)Pro2Cmd[5] & 0x00FF;
	Si446xCmd.PART_INFO.CUSTOMER = Pro2Cmd[6];
	Si446xCmd.PART_INFO.ROMID = Pro2Cmd[7];
	return ctsVal == 0xFF;
}

/*!
 * Get the Interrupt status/pending flags form the radio and clear flags when writing a 0 to them
 *
 * @param PH_CLR_PEND     Packet Handler pending flags clear.
 * @param MODEM_CLR_PEND  Modem Status pending flags clear.
 * @param CHIP_CLR_PEND   Chip State pending flags clear.
 */
bool Si446x::si446x_get_int_status(byte PH_CLR_PEND, byte MODEM_CLR_PEND, byte CHIP_CLR_PEND)
{
	Pro2Cmd[0] = SI446X_CMD_ID_GET_INT_STATUS;
	Pro2Cmd[1] = PH_CLR_PEND;
	Pro2Cmd[2] = MODEM_CLR_PEND;
	Pro2Cmd[3] = CHIP_CLR_PEND;

	byte ctsVal = radio_comm_SendCmdGetResp(SI446X_CMD_ARG_COUNT_GET_INT_STATUS,
											Pro2Cmd,
											SI446X_CMD_REPLY_COUNT_GET_INT_STATUS,
											Pro2Cmd);

	Si446xCmd.GET_INT_STATUS.INT_PEND = Pro2Cmd[0];
	Si446xCmd.GET_INT_STATUS.INT_STATUS = Pro2Cmd[1];
	Si446xCmd.GET_INT_STATUS.PH_PEND = Pro2Cmd[2];
	Si446xCmd.GET_INT_STATUS.PH_STATUS = Pro2Cmd[3];
	Si446xCmd.GET_INT_STATUS.MODEM_PEND = Pro2Cmd[4];
	Si446xCmd.GET_INT_STATUS.MODEM_STATUS = Pro2Cmd[5];
	Si446xCmd.GET_INT_STATUS.CHIP_PEND = Pro2Cmd[6];
	Si446xCmd.GET_INT_STATUS.CHIP_STATUS = Pro2Cmd[7];
	return ctsVal == 0xFF;
}

/*!
 * This function is used to load all properties and commands with a list of NULL terminated commands.
 * Before this function @si446x_reset should be called.
 * RF_POWER_UP : 15.7ms (IRQ becomes low when this command is finished)
 * RF_IRCAL : 107ms
 */
byte Si446x::si446x_configuration_init(const byte *pSetPropCmd)
{
	byte col;
	byte numOfBytes;

	/* While cycle as far as the pointer points to a command */
	while (*pSetPropCmd != 0x00)
	{
		/* Commands structure in the array:
     * --------------------------------
     * LEN | <LEN length of data>
     */

		numOfBytes = *pSetPropCmd++;

		if (numOfBytes > 16u)
		{
			/* Number of command bytes exceeds maximal allowable length */
			return SI446X_COMMAND_ERROR;
		}

		for (col = 0u; col < numOfBytes; col++)
		{
			Pro2Cmd[col] = *pSetPropCmd;
			pSetPropCmd++;
		}

		if (radio_comm_SendCmdGetResp(numOfBytes, Pro2Cmd, 0, 0) != 0xFF)
		{
			/* Timeout occured */
			return SI446X_CTS_TIMEOUT;
		}

		if (Module::digitalRead(_mod->getIrq()) == LOW)
		{
			/* Get and clear all interrupts.  An error has occured... */
			si446x_get_int_status(0, 0, 0);
			if (Si446xCmd.GET_INT_STATUS.CHIP_PEND & SI446X_CMD_GET_CHIP_STATUS_REP_CHIP_PEND_CMD_ERROR_PEND_MASK)
			{
				return SI446X_COMMAND_ERROR;
			}
		}
	}

	return SI446X_SUCCESS;
}

/*!
 * Send SET_PROPERTY command to the radio.
 *
 * @param GROUP       Property group.
 * @param NUM_PROPS   Number of property to be set. The properties must be in ascending order
 *                    in their sub-property aspect. Max. 12 properties can be set in one command.
 * @param START_PROP  Start sub-property address.
 */
void Si446x::si446x_set_property(byte GROUP, byte NUM_PROPS, byte START_PROP, void *values, byte len)
{
	Pro2Cmd[0] = SI446X_CMD_ID_SET_PROPERTY;
	Pro2Cmd[1] = GROUP;
	Pro2Cmd[2] = NUM_PROPS;
	Pro2Cmd[3] = START_PROP;

	// Copy values into data, starting at index 4
	memcpy(Pro2Cmd + 4, values, len);

	radio_comm_SendCmd(len + 4, Pro2Cmd);
}

/*!
 * Reads the Fast Response Registers starting with B register into @Si446xCmd union.
 *
 * @param respByteCount Number of Fast Response Registers to be read.
 */
byte Si446x::si446x_get_fastResponse(byte reg)
{
	radio_comm_ReadData(reg, 0, 1, Pro2Cmd);
	return Pro2Cmd[0];
}

/*!
 * Issue a change state command to the radio.
 *
 * @param NEXT_STATE1 Next state.
 */
void Si446x::si446x_change_state(byte NEXT_STATE1)
{
    Pro2Cmd[0] = SI446X_CMD_ID_CHANGE_STATE;
    Pro2Cmd[1] = NEXT_STATE1;

    radio_comm_SendCmd( SI446X_CMD_ARG_COUNT_CHANGE_STATE, Pro2Cmd );
}

/*!
 * Resets the RX/TX FIFO. Does not read back anything from TX/RX FIFO
 *
 */
void Si446x::si446x_fifo_info_fast_reset(byte FIFO)
{
    Pro2Cmd[0] = SI446X_CMD_ID_FIFO_INFO;
    Pro2Cmd[1] = FIFO;

    radio_comm_SendCmd( 2, Pro2Cmd );
}

/*!
 * Reads the RX FIFO content from the radio.
 *
 * @param numBytes  Data length to be read.
 * @param pRxData   Pointer to the buffer location.
 */
void Si446x::si446x_read_rx_fifo(byte numBytes, byte* pRxData)
{
  radio_comm_ReadData( SI446X_CMD_ID_READ_RX_FIFO, 0, numBytes, pRxData );
}

/*!
 * The function can be used to load data into TX FIFO.
 *
 * @param numBytes  Data length to be load.
 * @param pTxData   Pointer to the data (U8*).
 */
void Si446x::si446x_write_tx_fifo(byte numBytes, byte* pTxData)
{
  radio_comm_WriteData( SI446X_CMD_ID_WRITE_TX_FIFO, 0, numBytes, pTxData );
}

/*!
 * Sends START_RX command to the radio.
 *
 * @param CHANNEL     Channel number.
 * @param CONDITION   Start RX condition.
 * @param RX_LEN      Payload length (exclude the PH generated CRC).
 * @param NEXT_STATE1 Next state when Preamble Timeout occurs.
 * @param NEXT_STATE2 Next state when a valid packet received.
 * @param NEXT_STATE3 Next state when invalid packet received (e.g. CRC error).
 */
void Si446x::si446x_start_rx(byte CHANNEL, byte CONDITION, word RX_LEN, byte NEXT_STATE1, byte NEXT_STATE2, byte NEXT_STATE3)
{
    Pro2Cmd[0] = SI446X_CMD_ID_START_RX;
    Pro2Cmd[1] = CHANNEL;
    Pro2Cmd[2] = CONDITION;
    Pro2Cmd[3] = highByte(RX_LEN);
    Pro2Cmd[4] = lowByte(RX_LEN);
    Pro2Cmd[5] = NEXT_STATE1;
    Pro2Cmd[6] = NEXT_STATE2;
    Pro2Cmd[7] = NEXT_STATE3;

    radio_comm_SendCmd( SI446X_CMD_ARG_COUNT_START_RX, Pro2Cmd );
}

/*! Sends START_TX command to the radio.
 *
 * @param CHANNEL   Channel number.
 * @param CONDITION Start TX condition.
 * @param TX_LEN    Payload length (exclude the PH generated CRC).
 */
void Si446x::si446x_start_tx(byte CHANNEL, byte CONDITION, word TX_LEN)
{
    Pro2Cmd[0] = SI446X_CMD_ID_START_TX;
    Pro2Cmd[1] = CHANNEL;
    Pro2Cmd[2] = CONDITION;
    Pro2Cmd[3] = highByte(TX_LEN);
    Pro2Cmd[4] = lowByte(TX_LEN);

    radio_comm_SendCmd( 5, Pro2Cmd );
}
