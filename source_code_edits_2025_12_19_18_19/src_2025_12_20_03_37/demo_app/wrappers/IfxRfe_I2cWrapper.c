/*
 * (c) (2022-2023), Infineon Technologies AG, or an affiliate of Infineon Technologies AG. All rights reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or
 * the company in which ordinary course of business you are acting and (ii)
 * Infineon Technologies AG or its licensees.
 */
#include "IfxRfe_I2cWrapper.h"
#include <stddef.h>

/******************************************************************************/
/*-------------------------Local variables------------------------------------*/
/******************************************************************************/
static IfxRfe_i2cFunctions_t _fptrs = {NULL, NULL};


/******************************************************************************/
/*-------------------------Global Functions-----------------------------------*/
/******************************************************************************/
void InitI2cWrapper(IfxRfe_i2cFunctions_t fncs)
{
    _fptrs = fncs;
}

error_t Wrapper_I2cWrite(uint16_t devAddr, uint16_t length, const uint8_t buffer[])
{
    if (NULL == _fptrs.i2cWrite)
    {
        return IFXRFE_E_INVALID_CONFIGURATION;
    }

    return _fptrs.i2cWrite(devAddr, length, buffer);
}

error_t Wrapper_I2cRead(uint16_t devAddr, uint8_t prefix, uint16_t length, uint8_t buffer[])
{
    if (NULL == _fptrs.i2cRead)
    {
        return IFXRFE_E_INVALID_CONFIGURATION;
    }

    return _fptrs.i2cRead(devAddr, prefix, length, buffer);
}
// todo_672 add a read function