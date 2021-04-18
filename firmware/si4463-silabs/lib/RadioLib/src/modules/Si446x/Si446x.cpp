#include "Si446x.h"
#include "radio_config.h"
#include "si446x_prop.h"

Si446x::Si446x(Module *mod) : PhysicalLayer(SI4463_FREQUENCY_STEP_SIZE, SI4463_MAX_PACKET_LENGTH)
{
    _mod = mod;
}

int16_t Si446x::begin()
{
    // set module properties
    _mod->init(RADIOLIB_USE_SPI);
    Module::pinMode(_mod->getIrq(), INPUT);
    Module::digitalWrite(_mod->getRst(), HIGH);
    Module::pinMode(_mod->getRst(), OUTPUT);

    si446x_reset();
    byte Radio_Configuration_Data_Array[] = RADIO_CONFIGURATION_DATA_ARRAY;
    while (SI446X_SUCCESS != si446x_configuration_init(Radio_Configuration_Data_Array))
    {
        /* Error hook */
        delay(100); //todo find correct delay
        /* Power Up the radio chip */
        si446x_reset();
        delay(100);
    }
    // Read ITs, clear pending ones
    clearIRQFlags();
    si446x_part_info();
    // try to find the Si4463 chip
    if (Si446xCmd.PART_INFO.PART != 0x4463)
    {
        RADIOLIB_DEBUG_PRINTLN(F("No Si4463 found!"));
        _mod->term(RADIOLIB_USE_SPI);
        return (ERR_CHIP_NOT_FOUND);
    }
    RADIOLIB_DEBUG_PRINTLN(F("M\tSi4463"));

    sleep();
    enabledInterrupts[IRQ_PACKET] = SI446X_CMD_GET_PH_STATUS_REP_PH_PEND_PACKET_RX_PEND_MASK |
                                    SI446X_CMD_GET_PH_STATUS_REP_PH_PEND_CRC_ERROR_PEND_MASK;

    //   // clear POR interrupt
    //   clearIRQFlags();

    //   // configure settings not accessible by API
    int16_t state = ERR_NONE; // = config();
                              //   RADIOLIB_ASSERT(state);

    //   // configure publicly accessible settings
    //   state = setboolRate(br);
    //   RADIOLIB_ASSERT(state);

    //   state = setFrequencyDeviation(freqDev);
    //   RADIOLIB_ASSERT(state);

    //   state = setRxBandwidth(rxBw);
    //   RADIOLIB_ASSERT(state);

    //   state = setPreambleLength(preambleLen);
    //   RADIOLIB_ASSERT(state);

    //   uint8_t syncWord[] = {0x12, 0xAD};
    //   state = setSyncWord(syncWord, sizeof(syncWord));
    //   RADIOLIB_ASSERT(state);

    //   state = packetMode();
    //   RADIOLIB_ASSERT(state);

    //   state = setDataShaping(0);
    //   RADIOLIB_ASSERT(state);

    //   state = setEncoding(0);
    //   RADIOLIB_ASSERT(state);

    return (state);
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
void Si446x::setTxPower(uint8_t pwr)
{
    si446x_set_property(SI446X_PROP_GRP_ID_PA, 1, SI446X_PROP_GRP_INDEX_PA_PWR_LVL, &pwr, 1);
}

int16_t Si446x::sleep()
{
    if (getState() == SI446X_STATE_TX)
    {
        return 0;
    }
    si446x_change_state(SI446X_STATE_SLEEP);
    return SI446X_SUCCESS;
};

// Get current radio state
si446x_state_t Si446x::getState(void)
{
    uint8_t state = si446x_get_fastResponse(SI446X_CMD_ID_FRR_B_READ);
    switch (state)
    {
    case SI446X_STATE_TX_TUNE:
        state = SI446X_STATE_TX;
        break;
    case SI446X_STATE_RX_TUNE:
        state = SI446X_STATE_RX;
        break;
    case SI446X_STATE_READY2:
        state = SI446X_STATE_READY;
        break;
    default:
        break;
    }
    return (si446x_state_t)state;
}

int16_t Si446x::startReceive()
{
    si446x_change_state(SI446X_STATE_READY);
    si446x_fifo_info_fast_reset(SI446X_FIFO_CLEAR_RX | SI446X_FIFO_CLEAR_TX);
    //fix_invalidSync_irq(0);
    //Si446x_setupCallback(SI446X_CBS_INVALIDSYNC, 0);
    //setProperty(SI446X_PKT_FIELD_2_LENGTH_LOW, MAX_PACKET_LEN); // TODO ?
    si446x_get_int_status(0, 0, 0xFF);

    // TODO RX timeout to sleep if WUT LDC enabled
    word packetLength = 0; //variable length packets -> set to 0
    si446x_start_rx(_channel, 0, packetLength, SI446X_STATE_NOCHANGE, SI446X_STATE_READY, SI446X_STATE_SLEEP);
    return SI446X_SUCCESS;
}

void Si446x::setChannel(byte channel)
{
    _channel = channel;
}

size_t Si446x::getPacketLength(bool update)
{
    return _packetLength;
}

int16_t Si446x::receive(uint8_t *data, size_t len)
{
    // calculate timeout (500 ms + 400 full 64-byte packets at current bit rate)
    uint32_t timeout = 500000 + (1.0 / (_br * 1000.0)) * (SI4463_MAX_PACKET_LENGTH * 400.0);
    startReceive();
    // wait for packet reception or timeout
    uint32_t start = Module::micros();
    while (Module::digitalRead(_mod->getIrq()))
    {
        if (Module::micros() - start > timeout)
        {
            standby();
            clearIRQFlags();
            return (ERR_RX_TIMEOUT);
        }
    }
    clearIRQFlags();
    if (Si446xCmd.GET_INT_STATUS.PH_PEND & SI446X_CMD_GET_INT_STATUS_REP_PH_PEND_PACKET_RX_PEND_BIT)
    {
        //Packet RX interrupt pending
        if (Si446xCmd.GET_INT_STATUS.PH_STATUS & SI446X_CMD_GET_PH_STATUS_REP_PH_PEND_PACKET_RX_PEND_BIT)
        {
            //Valid packet in RX FIFO
            return readData(data, len);
        }
    }
    return ERR_PACKET_TOO_LONG;
}

void Si446x::clearIRQFlags()
{
    si446x_get_int_status(0u, 0u, 0u);
};

int16_t Si446x::readData(uint8_t *data, size_t len)
{
    byte length;
    //Read packet length (in case of variable length packet)
    si446x_read_rx_fifo(1, &length);
    _packetLength = length;
    //Read packet data
    si446x_read_rx_fifo(_packetLength, data);
    return ERR_NONE;
}
