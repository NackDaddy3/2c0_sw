/*
 * File: PlatformI2c.h
 * Description: Platform-dependant iRFE I2C API implementation for the NVIDIA Jetson
 * Copyright (c) 2025 Silicon Austria Labs GmbH
 */

#ifndef PLATFORMI2C_H
#define PLATFORMI2C_H

#include <stdint.h>

/*
 *   i2cId is derived from the /dev/i2c-A device file:
 *       i2cId = A
 */
int PlatformI2c_init(uint8_t const i2cId);
void PlatformI2c_deinit();

int PlatformI2c_write(uint16_t devAddr, uint16_t length, uint8_t const buffer[]);
int PlatformI2c_read(uint16_t devAddr, uint8_t prefix, uint16_t length, uint8_t buffer[]);

int PlatformI2c_writeRegister(uint8_t devAddr, uint8_t regAddr, uint8_t len, uint8_t const buffer[]);
int PlatformI2c_readRegister(uint8_t devAddr, uint8_t regAddr, uint8_t len, uint8_t *buffer);

#endif  // PLATFORMI2C_H