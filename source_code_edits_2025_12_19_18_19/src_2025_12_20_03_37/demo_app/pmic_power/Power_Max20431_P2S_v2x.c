/*
 * (c) (2022-2023), Infineon Technologies AG, or an affiliate of Infineon Technologies AG. All rights reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or
 * the company in which ordinary course of business you are acting and (ii)
 * Infineon Technologies AG or its licensees.
 */
#include "Power_Max20431_P2S_v2x.h"
#include "IfxRfe_Crc8.h"
#include "IfxRfe_GpioWrapper.h"
#include "IfxRfe_I2cWrapper.h"
#include "IfxRfe_Logger.h"
#include "IfxRfe_TimeWrapper.h"

/******************************************************************************/
/*-------------------------Local function prototypes--------------------------*/
/******************************************************************************/
static error_t waitForPin(uint16_t pinId, uint32_t timeoutMs);
static uint8_t devAddressRWBit(uint16_t devAddr, uint8_t readWriteBit);
static error_t max20431_I2cWrite(uint8_t regAddr, const uint8_t value);

/******************************************************************************/
/*-------------------------Local variables------------------------------------*/
/******************************************************************************/
PmicSupplyGpios_t _supplyGpios;
bool _initialized = false;

/******************************************************************************/
/*-------------------------Global Functions-----------------------------------*/
/******************************************************************************/

static void disablePmic()
{
    Wrapper_GpioSet(_supplyGpios.gpioEnPmic, false);
}

error_t Max20431_powerInit(PmicSupplyGpios_t gpios, IfxRfe_i2cFunctions_t i2cFuncs)
{
    InitI2cWrapper(i2cFuncs);
    _supplyGpios = gpios;
    _initialized = true;
    return IFXRFE_E_SUCCESS;
}

error_t Max20431_powerUp()
{
    if (false == _initialized)
    {
        return IFXRFE_E_NOT_INITIALIZED;
    }

    // enable and configure PMIC
    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioSet(_supplyGpios.gpioEnPmic, true));

    // PMIC power up sequence > ~2.1ms
    // see MAX20431 datasheet, timing, page 21
    Wrapper_WaitTime(10);
    const uint16_t pinTimeOutMs = 100U;

    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(CONFIGE, 0));  // Disable all voltage outputs

    // enable VOUT1
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(PINMAP1, 0x01));  // set up voltage monitor to check voltage VOUT1
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(CONFIGE, 0x01));  // turn on VOUT1
    IFXRFE_CLEAN_RETURN_ON_ERROR(waitForPin(_supplyGpios.gpioNrstPmic, pinTimeOutMs), disablePmic());

    // set output voltages
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(VOUT2, 104));  // Set VOUT2 to 1.8V
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(VOUT4, 32));   // Set VOUT4 to 0.9V

    // enable VOUT 2
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(PINMAP1, (0x01 | 0x02)));  // set up voltage monitor to check voltage VOUT1 and VOUT2
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(CONFIGE, (0x01 | 0x02)));  // turn on VOUT2
    IFXRFE_CLEAN_RETURN_ON_ERROR(waitForPin(_supplyGpios.gpioNrstPmic, pinTimeOutMs), disablePmic());

    // on a V2.2 mainboard the 1V8 comes from PMIC VOUT2 and 1V3 LDO has to be enabled instead
    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioSet(_supplyGpios.gpioEnLdo1V3, true));  // enable 1V3 LDO
    IFXRFE_CLEAN_RETURN_ON_ERROR(waitForPin(_supplyGpios.gpioPgLdo1V3, pinTimeOutMs), disablePmic());

    // on V2.2 board the VOUT3 (fix 5V) is used, on V1.x it is unused
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(PINMAP1, (0x01 | 0x02 | 0x04)));  // set up voltage monitor to check voltage VOUT1, VOUT2 and VOUT3
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(CONFIGE, (0x01 | 0x02 | 0x04)));  // turn on VOUT3
    IFXRFE_CLEAN_RETURN_ON_ERROR(waitForPin(_supplyGpios.gpioNrstPmic, pinTimeOutMs), disablePmic());

    // enable VOUT4
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(PINMAP1, (0x01 | 0x02 | 0x04 | 0x08)));  // set up voltage monitor to check voltage VOUT1, VOUT2, VOUT3 and VOUT4
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(CONFIGE, (0x01 | 0x02 | 0x04 | 0x08)));  // turn on VOUT4
    IFXRFE_CLEAN_RETURN_ON_ERROR(waitForPin(_supplyGpios.gpioNrstPmic, pinTimeOutMs), disablePmic());

    return IFXRFE_E_SUCCESS;
}

error_t Max20431_powerDown()
{
    if (false == _initialized)
    {
        return IFXRFE_E_NOT_INITIALIZED;
    }

    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(CONFIGE, (0x01 | 0x02 | 0x04)));  // turn off VOUT4
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(CONFIGE, (0x01 | 0x02)));         // turn off VOUT3

    // on a V2.2 mainboard the 1V8 comes from PMIC VOUT2 and 1V3 LDO has to be disabled instead
    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioSet(_supplyGpios.gpioEnLdo1V3, false));  // disable 1V3 LDO
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(CONFIGE, (0x01)));                 // turn off VOUT2

    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioSet(_supplyGpios.gpioEnPmic, false));  // disable PMIC
    return IFXRFE_E_SUCCESS;
}

/******************************************************************************/
/*-------------------------Local helper functions-----------------------------*/
/******************************************************************************/
static error_t waitForPin(uint16_t pinId, uint32_t timeoutMs)
{
    int64_t deadline;
    bool state = false;
    int64_t now;

    IFXRFE_RETURN_ON_ERROR(Wrapper_GetDeadLine(timeoutMs, &deadline));

    IFXRFE_RETURN_ON_ERROR(Wrapper_Now(&now));  //check once here, if no error returned,
        //function is configured correctly and return value does not have to be checked in the loop
    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioGet(pinId, &state));

    while ((false == state) && (now < deadline))
    {
        Wrapper_GpioGet(pinId, &state);
        Wrapper_Now(&now);
    }

    if (false == state)
    {
        return IFXRFE_E_TIMEOUT;
    }
    return IFXRFE_E_SUCCESS;
}
static uint8_t devAddressRWBit(uint16_t devAddr, uint8_t readWriteBit)
{
    uint8_t devAddress7bit = IFXRFE_I2C_MASK_DEV_ADDR_7BIT(devAddr);
    return (devAddress7bit << 1) | readWriteBit;
}

static error_t max20431_I2cWrite(uint8_t regAddr, const uint8_t value)
{
    const uint8_t dataForCrc[] = {devAddressRWBit(PMIC_MAX20431_I2C_ADDR, 0U), regAddr, value};
    const uint8_t crc          = crc8SmBus(dataForCrc, (uint16_t)(sizeof(dataForCrc)));

    const uint8_t payload[] = {regAddr, value, crc};
    IFXRFE_RETURN_ON_ERROR(Wrapper_I2cWrite(PMIC_MAX20431_I2C_ADDR, sizeof(payload), payload));

    return IFXRFE_E_SUCCESS;
}
