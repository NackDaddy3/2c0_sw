/*
 * (c) (2022-2023), Infineon Technologies AG, or an affiliate of Infineon Technologies AG. All rights reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or
 * the company in which ordinary course of business you are acting and (ii)
 * Infineon Technologies AG or its licensees.
 */
#ifndef POWER_MAX20431_H
#define POWER_MAX20431_H 1

#include "IfxRfe_ErrorDefinitions.h"
#include "IfxRfe_I2cConfig.h"
#include <stdbool.h>
#include <stdint.h>

#define PMIC_MAX204030PEC
#define PMIC_MAX20431_I2C_ADDR 0x38

typedef struct
{
    uint16_t gpioEnPmic3V3;
    uint16_t gpioNrstPmic;
    uint16_t gpioEnPmic;

    uint16_t gpioEnLdo1V8;
    uint16_t gpioPgLdo1V8;

    uint16_t gpioEnLdo0V9Rf;
    uint16_t gpioPgLdo0V9Rf;

    uint16_t gpioEnLdo0V9PA;
    uint16_t gpioPgLdo0V9PA;

    uint16_t gpioEnLdo0V9D;
    uint16_t gpioPgLdo0V9D;
} PmicSupplyGpios_t;

typedef enum
{
    CID     = 0x00,
    CONFIG1 = 0x01,
    CONFIGE = 0x03,
    FPSCFG  = 0x05,
    PINMAP1 = 0x07,
    VOUT2   = 0x0E,
    VOUT4   = 0x0F,
    VIN5    = 0x10,
    VIN6    = 0x11,
    WDCFG2  = 0x14,
    WDPROT  = 0x16,
} PmicMax2043xI2cReg_t;

///
/// \brief Power Module Initialize function
/// \param gpios structure containing the list of gpios for the pmic control
/// \param i2cFuncs structure containing the i2c access function pointers
/// \return IFXRFE_E_SUCCESS on success
error_t Max20431_powerInit(PmicSupplyGpios_t gpios, IfxRfe_i2cFunctions_t i2cFuncs);

/// \brief Power-Up Sequence PMIC MAX20431Pec
/// \retval IFXRFE_E_SUCCESS on success
/// \retval IFXRFE_E_NOT_INITIALIZED if function called before successfull \ref Max20431_powerInit
/// \retval IFXRFE_E_TIMEOUT if any of the power-good pins were not set in time
/// \retval IFXRFE_E_INVALID_CONFIGURATION if pin configuration is invalid
/// \note error code from \ref Wrapper_GpioSet on error
/// \note error code from \ref Wrapper_I2cWriteWith8BitPrefix on error
error_t Max20431_powerUp();

/// \brief Power-Down Sequence PMIC MAX20431Pec
/// \retval IFXRFE_E_SUCCESS on success
/// \retval IFXRFE_E_NOT_INITIALIZED if function called before successfull \ref Max20431_powerInit
/// \note error code from \ref Wrapper_GpioSet on error
/// \note error code from \ref Wrapper_I2cWriteWith8BitPrefix on error
error_t Max20431_powerDown();

#endif /* POWER_MAX20431_H */
