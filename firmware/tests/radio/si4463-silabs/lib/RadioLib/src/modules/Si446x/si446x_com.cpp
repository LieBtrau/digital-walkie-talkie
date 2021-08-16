#include "Si446x.h"

/*!
 * Sends a command to the radio chip and gets a response
 *
 * @param cmdByteCount  Number of bytes in the command to send to the radio device
 * @param pCmdData      Pointer to the command data
 * @param respByteCount Number of bytes in the response to fetch
 * @param pRespData     Pointer to where to put the response data
 *
 * @return CTS value
 */
byte Si446x::radio_comm_SendCmdGetResp(byte cmdByteCount, byte *pCmdData, byte respByteCount, byte *pRespData)
{
    radio_comm_SendCmd(cmdByteCount, pCmdData);
    return radio_comm_GetResp(respByteCount, pRespData);
}

/*!
 * Gets a command response from the radio chip
 *
 * @param byteCount     Number of bytes to get from the radio chip
 * @param pData         Pointer to where to put the data
 *
 * @return CTS value
 */
byte Si446x::radio_comm_GetResp(byte byteCount, byte *pData)
{
    byte ctsVal = 0u;
    word errCnt = RADIO_CTS_TIMEOUT;
    byte response[byteCount + 1];
    while (errCnt != 0) //wait until radio IC is ready with the data
    {
        ctsVal =_mod->SPIgetRegValue(SI446X_CMD_ID_READ_CMD_BUFF);
        if (ctsVal == 0xFF)
        {
            if (byteCount > 1)
            {
                /*We're starting a new SPI-transfer here because the library doesn't allow us to control the /CS line.
                 *In the ideal case, when we read 0xFF, we can continue and read the result without toggling /CS and restarting the SPI-transfer.
                 */
             _mod->SPIreadRegisterBurst(SI446X_CMD_ID_READ_CMD_BUFF, byteCount + 1, response);
               memcpy(pData, response + 1, byteCount);
            }
            break;
        }
        errCnt--;
    }
    if (errCnt == 0)
    {
        while (1)
        {
/* ERROR!!!!  CTS should never take this long. */
#ifdef RADIO_COMM_ERROR_CALLBACK
            RADIO_COMM_ERROR_CALLBACK();
#endif
        }
    }
    if (ctsVal == 0xFF)
    {
        ctsWentHigh = 1;
    }
    return ctsVal;
}

/*!
 * Sends a command to the radio chip
 *
 * @param byteCount     Number of bytes in the command to send to the radio device
 * @param pData         Pointer to the command to send.
 */
void Si446x::radio_comm_SendCmd(byte byteCount, byte *pData)
{
    while (!ctsWentHigh)
    {
        radio_comm_PollCTS();
    }
    _mod->SPIwriteRegisterBurst(*pData, pData + 1, byteCount - 1);
    delayMicroseconds(20);
    ctsWentHigh = 0;
}

/*!
 * Send a command to the radio chip
 *
 * @param cmd           Command ID
 * @param pollCts       Set to poll CTS
 * @param byteCount     Number of bytes to send to the radio chip
 * @param pData         Pointer to where to get the data
 */
void Si446x::radio_comm_WriteData(byte cmd, bool pollCts, byte byteCount, byte *pData)
{
    if (pollCts)
    {
        while (!ctsWentHigh)
        {
            radio_comm_PollCTS();
        }
    }
    _mod->SPIwriteRegisterBurst(cmd, pData, byteCount);
    ctsWentHigh = 0;
}


/*!
 * Gets a command response from the radio chip
 *
 * @param cmd           Command ID
 * @param pollCts       Set to poll CTS
 * @param byteCount     Number of bytes to get from the radio chip.
 * @param pData         Pointer to where to put the data.
 */
void Si446x::radio_comm_ReadData(byte cmd, bool pollCts, byte byteCount, byte *pData)
{
    if (pollCts)
    {
        //while (!ctsWentHigh)
        {
            radio_comm_PollCTS();
        }
    }
    _mod->SPIreadRegisterBurst(cmd, byteCount, pData);
    ctsWentHigh = 0;
}

/*!
 * Waits for CTS to be high
 *
 * @return CTS value
 */
byte Si446x::radio_comm_PollCTS(void)
{
#ifdef RADIO_USER_CFG_USE_GPIO1_FOR_CTS
    while (!radio_hal_Gpio1Level())
    {
        /* Wait...*/
    }
    ctsWentHigh = 1;
    return 0xFF;
#else
    byte ctsVal;
    return radio_comm_GetResp(1, &ctsVal);
#endif
}

/**
 * Clears the CTS state variable.
 */
void Si446x::radio_comm_ClearCTS()
{
    ctsWentHigh = 0;
}
