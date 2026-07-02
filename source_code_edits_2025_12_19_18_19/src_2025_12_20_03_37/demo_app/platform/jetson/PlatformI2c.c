/*
 * File: PlatformI2c.c
 * Description: Platform-dependant iRFE I2C API implementation for the NVIDIA Jetson
 * Copyright (c) 2025 Silicon Austria Labs GmbH
 */

#include <errno.h>
#include <fcntl.h>
#include <i2c/smbus.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "IfxRfe_ErrorDefinitions.h"

#include "PlatformI2c.h"
#include "PlatformLogCallbacks.h"
#include "Util.h"

static int gI2cFd = -1;

int PlatformI2c_init(uint8_t const i2cId)
{
    DBG_PRINTF_L1("DEBUG from %s() at %s:%d:\n", __func__, __FILE__, __LINE__);

    if (gI2cFd >= 0)
    {
        char msg[32];
        snprintf(msg, 32, "I2C: already initialized!");
        PlatformLogCallbacks_error(msg);
        return IFXRFE_E_FAILED;
    }

    DBG_PRINTF_L1("    i2cId = %d\n", i2cId);

    // Generate device file name
    char i2cdevname[32];
    sprintf(i2cdevname, "/dev/i2c-%d", i2cId);

    DBG_PRINTF_L1("    i2cdevname = %s\n", i2cdevname);

    gI2cFd = open(i2cdevname, O_RDWR);
    if (gI2cFd < 0)
    {
        char msg[128];
        snprintf(msg, 128, "I2C: open() returned %d, errno = %d (%s)", gI2cFd, errno, strerror(errno));
        PlatformLogCallbacks_error(msg);
        return IFXRFE_E_FAILED;
    }

    // check adapter functionality
    unsigned long funcs;
    int sysret = ioctl(gI2cFd, I2C_FUNCS, &funcs);
    if (sysret < 0)
    {
        char msg[128];
        snprintf(msg, 128, "I2C: ioctl() for I2C_FUNCS returned %d, errno = %d (%s)", sysret, errno, strerror(errno));
        PlatformLogCallbacks_error(msg);
        PlatformI2c_deinit();
        return IFXRFE_E_FAILED;
    }

    if ((funcs & I2C_FUNC_SMBUS_I2C_BLOCK) != I2C_FUNC_SMBUS_I2C_BLOCK)
    {
        char msg[128];
        snprintf(msg, 128, "I2C adapter is missing functionality: I2C_FUNC_SMBUS_I2C_BLOCK");
        PlatformLogCallbacks_error(msg);
        PlatformI2c_deinit();
        return IFXRFE_E_FAILED;
    }

    return IFXRFE_E_SUCCESS;
}

void PlatformI2c_deinit()
{
    if (gI2cFd >= 0)
    {
        close(gI2cFd);
        gI2cFd = -1;
    }
}

int PlatformI2c_write(uint16_t devAddr, uint16_t length, uint8_t const buffer[])
{
    DBG_PRINTF_L1("DEBUG from %s() at %s:%d:\n", __func__, __FILE__, __LINE__);
    DBG_PRINTF_L1("    devAddr = 0x%02X\n", devAddr);
    DBG_PRINTF_L1("    length = %d\n", length);
    if (buffer != NULL && length > 0)
    {
        DBG_PRINTF_L1("    buffer:\n");
        DBG_PRINTF_L1("        ");
        for (size_t i = 0; i < length; i++)
        {
            if (i == 0)
            {
                DBG_PRINTF_L1("0x%02X", buffer[i]);
            }
            else
            {
                DBG_PRINTF_L1(", 0x%02X", buffer[i]);
            }
        }
        DBG_PRINTF_L1("\n");
    }

    // Check initialisation state
    if (gI2cFd < 0)
    {
        char msg[32];
        snprintf(msg, 32, "I2C not initialized");
        PlatformLogCallbacks_error(msg);
        return IFXRFE_E_NOT_INITIALIZED;
    }

    // Length boundary check
    // https://manpages.debian.org/unstable/i2c-tools/libi2c.3.en.html
    // Block length is limited to 32 Bytes (+1 == 33, including the register address)
    if (length > 33)
    {
        char msg[64];
        snprintf(msg, 64, "I2C write buffer length not supported: %d", length);
        PlatformLogCallbacks_error(msg);
        return IFXRFE_E_NOT_SUPPORTED;
    }

    // Set slave address
    int sysret = ioctl(gI2cFd, I2C_SLAVE, devAddr);
    if (sysret < 0)
    {
        char msg[128];
        snprintf(msg, 128, "I2C could not set slave address: ioctl() for I2C_SLAVE returned %d, errno = %d (%s)", sysret, errno, strerror(errno));
        PlatformLogCallbacks_error(msg);
        return IFXRFE_E_FAILED;
    }

    // Send data
    // buffer[0] is usually the register address
    // buffer[1+] contains the data to write
    sysret = i2c_smbus_write_i2c_block_data(gI2cFd, buffer[0], length - 1, &buffer[1]);
    if (sysret < 0)
    {
        char msg[128];
        snprintf(msg, 128, "I2C could not write data: i2c_smbus_write_i2c_block_data() returned %d, errno = %d (%s)", sysret, errno, strerror(errno));
        PlatformLogCallbacks_error(msg);
        return IFXRFE_E_FAILED;
    }

    return IFXRFE_E_SUCCESS;
}

int PlatformI2c_read(uint16_t devAddr, uint8_t prefix, uint16_t length, uint8_t buffer[])
{
    DBG_PRINTF_L1("DEBUG from %s() at %s:%d:\n", __func__, __FILE__, __LINE__);
    DBG_PRINTF_L1("    devAddr = 0x%02X\n", devAddr);
    DBG_PRINTF_L1("    prefix = 0x%02X\n", prefix);
    DBG_PRINTF_L1("    length = %d\n", length);

    // Check initialisation state
    if (gI2cFd < 0)
    {
        char msg[32];
        snprintf(msg, 32, "I2C not initialized");
        PlatformLogCallbacks_error(msg);
        return IFXRFE_E_NOT_INITIALIZED;
    }

    // Length boundary check
    // https://manpages.debian.org/unstable/i2c-tools/libi2c.3.en.html
    // Block length is limited to 32 Bytes
    if (length > 32)
    {
        char msg[64];
        snprintf(msg, 64, "I2C read buffer length not supported: %d", length);
        PlatformLogCallbacks_error(msg);
        return IFXRFE_E_NOT_SUPPORTED;
    }

    // Set slave address
    int sysret = ioctl(gI2cFd, I2C_SLAVE, devAddr);
    if (sysret < 0)
    {
        char msg[128];
        snprintf(msg, 128, "I2C could not set slave address: ioctl() for I2C_SLAVE returned %d, errno = %d (%s)", sysret, errno, strerror(errno));
        PlatformLogCallbacks_error(msg);
        return IFXRFE_E_FAILED;
    }

    sysret = i2c_smbus_read_i2c_block_data(gI2cFd, prefix, length, buffer);
    if (sysret < 0)
    {
        char msg[128];
        snprintf(msg, 128, "I2C could not read data: i2c_smbus_read_i2c_block_data() returned %d, errno = %d (%s)", sysret, errno, strerror(errno));
        PlatformLogCallbacks_error(msg);
        return IFXRFE_E_FAILED;
    }

    if (buffer != NULL && length > 0)
    {
        DBG_PRINTF_L1("    read_buffer:\n");
        DBG_PRINTF_L1("        ");
        for (size_t i = 0; i < length; i++)
        {
            if (i == 0)
            {
                DBG_PRINTF_L1("0x%02X", buffer[i]);
            }
            else
            {
                DBG_PRINTF_L1(", 0x%02X", buffer[i]);
            }
        }
        DBG_PRINTF_L1("\n");
    }

    return IFXRFE_E_SUCCESS;
}

int PlatformI2c_writeRegister(uint8_t devAddr, uint8_t regAddr, uint8_t len, uint8_t const buffer[])
{    
    DBG_PRINTF_L1("DEBUG from %s() at %s:%d:\n", __func__, __FILE__, __LINE__);
    DBG_PRINTF_L1("    devAddr = 0x%02X\n", devAddr);
    DBG_PRINTF_L1("    regAddr = 0x%02X\n", regAddr);
    DBG_PRINTF_L1("    len = %d\n", len);

    if (len > 32)
    {
        char msg[64];
        snprintf(msg, 64, "I2C write buffer length not supported: %d", len);
        PlatformLogCallbacks_error(msg);
        return IFXRFE_E_NOT_SUPPORTED;
    }

    uint8_t write_buffer[33];
    write_buffer[0] = regAddr;
    for (size_t i = 0; i < len; i++)
    {
        write_buffer[i + 1] = buffer[i];
    }
    return PlatformI2c_write(devAddr, len + 1, write_buffer);
}

int PlatformI2c_readRegister(uint8_t devAddr, uint8_t regAddr, uint8_t len, uint8_t *buffer)
{
    DBG_PRINTF_L1("DEBUG from %s() at %s:%d:\n", __func__, __FILE__, __LINE__);
    DBG_PRINTF_L1("    devAddr = 0x%02X\n", devAddr);
    DBG_PRINTF_L1("    regAddr = 0x%02X\n", regAddr);
    DBG_PRINTF_L1("    len = %d\n", len);

    return PlatformI2c_read(devAddr, regAddr, len, buffer);
}