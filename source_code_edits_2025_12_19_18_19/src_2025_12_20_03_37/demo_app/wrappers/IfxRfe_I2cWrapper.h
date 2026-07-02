/*
 * (c) (2022-2023), Infineon Technologies AG, or an affiliate of Infineon Technologies AG. All rights reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or
 * the company in which ordinary course of business you are acting and (ii)
 * Infineon Technologies AG or its licensees.
 */
#ifndef IFXRFE_I2CWRAPPER_H
#define IFXRFE_I2CWRAPPER_H 1

#include "IfxRfe_ErrorDefinitions.h"
#include "IfxRfe_I2cConfig.h"
#include <stdbool.h>
#include <stdint.h>

#define IFXRFE_I2C_MASK_DEV_ADDR_7BIT(devAddr) ((devAddr)&0x007Fu)

/// \brief Initialize the I2C wrapper
/// \param fncs The collection of the i2c access functions to use
void InitI2cWrapper(IfxRfe_i2cFunctions_t fncs);

/// \brief I2C write function
/// \param devAddr I2C address
/// \param length Nr of bytes to be written
/// \param buffer Data to be written
/// \retval IFXRFE_E_SUCCESS if ok
/// \retval IFXRFE_E_NOT_CONFIGURED if function pointer configured in \ref InitI2cWrapper is NULL
/// \note error code returned from the function pointer otherwise
error_t Wrapper_I2cWrite(uint16_t devAddr, uint16_t length, const uint8_t buffer[]);


/// \brief I2C read function
/// \param devAddr I2C address
/// \param prefix Prefix byte to be sent before reading
/// \param length Nr of bytes to be read
/// \param buffer Data read
/// \retval IFXRFE_E_SUCCESS if ok
/// \retval IFXRFE_E_NOT_CONFIGURED if function pointer configured in \ref InitI2cWrapper is NULL
/// \note error code returned from the function pointer otherwise
error_t Wrapper_I2cRead(uint16_t devAddr, uint8_t prefix, uint16_t length, uint8_t buffer[]);
#endif
