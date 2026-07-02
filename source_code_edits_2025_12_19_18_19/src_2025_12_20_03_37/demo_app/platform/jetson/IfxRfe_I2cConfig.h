/*
 * (c) (2022-2023), Infineon Technologies AG, or an affiliate of Infineon Technologies AG. All rights reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or
 * the company in which ordinary course of business you are acting and (ii)
 * Infineon Technologies AG or its licensees.
 */
#ifndef IFXRFE_I2CCONFIG_H
#define IFXRFE_I2CCONFIG_H 1

#include <stdint.h>

typedef struct
{
    /// \brief I2C write function
    /// \param devAddr I2C device address
    /// \param length Nr of bytes to write
    /// \param buffer Data to be sent
    /// \return 0 if ok, error code in case of errors.
    ///         Error codes should be returned in the callbacks. Error code translation has to be handled by user.
    ///         \see IfxRfe_ErrorDefinitions.h for the error code definitions of the IRFE driver
    int (*i2cWrite)(uint16_t devAddr, uint16_t length, const uint8_t buffer[]);

    /// \brief I2C read function
    /// \param devAddr I2C device address
    /// \param prefix Prefix byte to be sent before reading
    /// \param length Nr of bytes to read
    /// \param buffer Data to be stored
    /// \return 0 if ok, error code in case of errors.
    ///         Error codes should be returned in the callbacks. Error code translation has to be handled by user.
    ///         \see IfxRfe_ErrorDefinitions.h for the error code definitions of the IRFE driver
    int (*i2cRead)(uint16_t devAddr, uint8_t prefix, uint16_t length, uint8_t buffer[]);
} IfxRfe_i2cFunctions_t;

#endif  //IFXRFE_I2CCONFIG_H