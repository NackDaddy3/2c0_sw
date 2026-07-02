/*
 * (c) (2022-2023), Infineon Technologies AG, or an affiliate of Infineon Technologies AG. All rights reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or
 * the company in which ordinary course of business you are acting and (ii)
 * Infineon Technologies AG or its licensees.
 */
#include "Power_Max20431_P2S_v1x.h"
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
    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioSet(_supplyGpios.gpioEnPmic3V3, true));
    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioSet(_supplyGpios.gpioEnPmic, true));

    // PMIC power up sequence > ~2.1ms
    // see MAX20431 datasheet, timing, page 21
    Wrapper_WaitTime(5);

    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(CONFIGE, 0));     // Disable all voltage outputs, except VOUT1
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(WDCFG2, 0));      // Disable watchdog
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(PINMAP1, 0x0B));  // ResetB mapped to VOUT1, VOUT2 and VOUT4
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(VIN5, 80));       // Set VIN5 monitoring to 1.8V ( VIN5=(1.8-0.8)/0.0125 )
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(VIN6, 8));        // Set VIN6 monitoring to 0.9V ( VIN6=(0.9-0.8)/0.0125 )
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(FPSCFG, 0));      // Power Sequ. Config (0: 2.5ms sequenced time slots for VOUT2..4 startup)
    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(CONFIGE, 0x02));  // Enable VOUT2 (2.3V)

    const uint16_t pinTimeOutMs = 100U;

    // Enable LDO's and Enable 3V3 switch according necessary sequence
    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioSet(_supplyGpios.gpioEnLdo1V8, true));  // enable 1V8 LDO
    IFXRFE_RETURN_ON_ERROR(waitForPin(_supplyGpios.gpioPgLdo1V8, pinTimeOutMs));

    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioSet(_supplyGpios.gpioEnPmic3V3, false));  // turn on 3V3 switch

    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(CONFIGE, 0x0A));  // Enable VOUT4 (1.3V) in addition to VOUT2 (2.3V)

    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioSet(_supplyGpios.gpioEnLdo0V9Rf, true));  // enable 0V9RF LDO
    IFXRFE_RETURN_ON_ERROR(waitForPin(_supplyGpios.gpioPgLdo0V9Rf, pinTimeOutMs));

    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioSet(_supplyGpios.gpioEnLdo0V9PA, true));  // enable 0V9PA LDO
    IFXRFE_RETURN_ON_ERROR(waitForPin(_supplyGpios.gpioPgLdo0V9PA, pinTimeOutMs));

    IFXRFE_RETURN_ON_ERROR(waitForPin(_supplyGpios.gpioNrstPmic, pinTimeOutMs));
    return IFXRFE_E_SUCCESS;
}

error_t Max20431_powerDown()
{
    if (false == _initialized)
    {
        return IFXRFE_E_NOT_INITIALIZED;
    }

    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioSet(_supplyGpios.gpioEnLdo0V9Rf, false));
    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioSet(_supplyGpios.gpioEnLdo0V9PA, false));
    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioSet(_supplyGpios.gpioEnPmic3V3, true));

    IFXRFE_RETURN_ON_ERROR(max20431_I2cWrite(CONFIGE, 0x02));  // disable VOUT4 (1.3V)

    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioSet(_supplyGpios.gpioEnLdo1V8, false));  // disable 1V8 LDO
    IFXRFE_RETURN_ON_ERROR(Wrapper_GpioSet(_supplyGpios.gpioEnPmic, false));    // disable PMIC
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
