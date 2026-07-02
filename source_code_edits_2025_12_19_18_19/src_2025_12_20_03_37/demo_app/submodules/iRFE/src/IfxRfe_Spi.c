/*
 * (c) (2022-2023), Infineon Technologies AG, or an affiliate of Infineon Technologies AG. All rights reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or
 * the company in which ordinary course of business you are acting and (ii)
 * Infineon Technologies AG or its licensees.
 */
#include "IfxRfe_Spi.h"

#include "IfxRfe_BuilderParser.h"
#include "IfxRfe_CtrxOpCodes.h"
#include "IfxRfe_Gpio.h"
#include "IfxRfe_Logger.h"
#include "IfxRfe_SpiWrapper.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/******************************************************************************/
/*-------------------------MACROS---------------------------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-------------------------Local Variables------------------------------------*/
/******************************************************************************/
static bool _crcCheckEnabled = false;  //A-step does not have crc calculation in the ROM-FW
                                       //has to be enabled after RAM-FW download

/******************************************************************************/
/******************************************************************************/
/*-------------------------Local Function Prototypes--------------------------*/
/******************************************************************************/

/******************************************************************************/
/*-------------------------Global Function Implementations--------------------*/
/******************************************************************************/

static error_t ResetKeepSel()
{
    return Wrapper_SpiWrite(0, NULL, false);
}

error_t SpiTransfer(uint32_t *wbuf, uint8_t wlen, uint32_t *rbuf, uint8_t *rlen)
{
    if ((NULL == wbuf) || (0 == wlen) || (NULL == rbuf) || (NULL == rlen))
    {
        return IFXRFE_E_MISSING_PARAMETER;
    }
    IFXRFE_RETURN_ON_ERROR(IfxRfe_waitForRftPin(IFXRFE_RFT_TIMEOUT_DEF));

    //get header first
    IFXRFE_CLEAN_RETURN_ON_ERROR(Wrapper_SpiTransfer(1, wbuf, rbuf, true), ResetKeepSel());

    //check response header crc
    if (_crcCheckEnabled)
    {
        IFXRFE_CLEAN_RETURN_ON_ERROR(CheckResponseHeaderCrc(rbuf[0]), ResetKeepSel());
    }

    uint8_t txLen       = wlen - 1;  //remaining length: if no payload, 0, otherwise payload + CRC32
    uint8_t rxLen       = GET_PAYLOAD_LEN(rbuf[0]);
    rxLen               = (0 == rxLen) ? rxLen : rxLen + 1;  //note the payload length does note contain the CRC32, has to be added (only if payload available)
    uint8_t transferLen = (txLen > rxLen) ? txLen : rxLen;
    if (rxLen + 1 > MAX_RES_PAYLOAD_WORDS + 2)  //CRC valid, but the length is too high (note: add 1 because the header was not included)
    {
        ResetKeepSel();
        return IFXRFE_E_UNEXPECTED_VALUE;
    }

    //reset the unused values not to send out anything wrong
    memset(&wbuf[txLen + 1], 0, (transferLen - txLen) * sizeof(uint32_t));  //note: not a problem if length is 0

    //finish transmission
    if (transferLen > 0)
    {
        IFXRFE_CLEAN_RETURN_ON_ERROR(Wrapper_SpiTransfer(transferLen, &wbuf[1], &rbuf[1], false), ResetKeepSel());
    }
    else
    {
        //both receive and transmit are header only
        ResetKeepSel();
    }
    *rlen = rxLen + 1;

    //check received data
    if ((rxLen != 0) && _crcCheckEnabled)
    {
        IFXRFE_RETURN_ON_ERROR(CheckResponsePayloadCrc(rbuf, *rlen));
    }

    return IFXRFE_E_SUCCESS;
}

error_t SpiSend(uint32_t *wbuf, uint8_t wlen)
{
    if ((NULL == wbuf) || (0 == wlen))
    {
        return IFXRFE_E_MISSING_PARAMETER;
    }

    IFXRFE_RETURN_ON_ERROR(IfxRfe_waitForRftPin(IFXRFE_RFT_TIMEOUT_DEF));

    IFXRFE_RETURN_ON_ERROR(Wrapper_SpiWrite(wlen, wbuf, false));

    return IFXRFE_E_SUCCESS;
}

error_t SpiReceive(uint32_t *rbuf, uint8_t *rlen)
{
    if ((NULL == rbuf) || (NULL == rlen))
    {
        return IFXRFE_E_MISSING_PARAMETER;
    }

    IfxRfe_ctrxRequest_t nop;
    nop.payloadLen = 0;
    IFXRFE_RETURN_ON_ERROR(FillHeaderAndCrc(IFXRFE_OPCODE_NOP, 0, &nop));

    //transfer NOP command, receive length will be determined by the received header
    IFXRFE_RETURN_ON_ERROR(SpiTransfer(nop.buffer, 1, rbuf, rlen));

    return IFXRFE_E_SUCCESS;
}

error_t SpiSendReceive(uint32_t *wbuf, uint8_t wlen, uint32_t *rbuf, uint8_t *rlen)
{
    IFXRFE_RETURN_ON_ERROR(SpiSend(wbuf, wlen));

    return SpiReceive(rbuf, rlen);
}


void EnableDisableCrcCheck(bool enabled)
{
    _crcCheckEnabled = enabled;
}

/******************************************************************************/
/*-------------------------Local Function Implementations---------------------*/
/******************************************************************************/
