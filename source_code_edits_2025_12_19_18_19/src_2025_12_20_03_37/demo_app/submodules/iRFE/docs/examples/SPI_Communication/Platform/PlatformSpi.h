/**
 * \addtogroup      PlatformSpi
 * @{
 */
#ifndef PLATFORM_SPI_H
#define PLATFORM_SPI_H 1

#include <stdbool.h>
#include <stdint.h>

#include "common/errors.h"
#include "PlatformSpiDefinition.h"
#include "universal/spi_definitions.h"


sr_t PlatformSpi_initialize(const PlatformSpiDefinition_Master *masters, uint8_t masterCount, const PlatformSpiDefinition_Device *devices, uint8_t deviceCount);

/******************************************************************************/
/*Interface Methods Declaration ----------------------------------------------*/
/******************************************************************************/
uint32_t PlatformSpi_getMaxTransfer(void);
sr_t PlatformSpi_configure(uint8_t devId, uint8_t flags, uint8_t wordSize, uint32_t speed);
sr_t PlatformSpi_write8(uint8_t devId, uint32_t count, const uint8_t buffer[], bool keepSel);
sr_t PlatformSpi_write16(uint8_t devId, uint32_t count, const uint16_t buffer[], bool keepSel);
sr_t PlatformSpi_write32(uint8_t devId, uint32_t count, const uint32_t buffer[], bool keepSel);
sr_t PlatformSpi_read8(uint8_t devId, uint32_t count, uint8_t buffer[], bool keepSel);
sr_t PlatformSpi_read16(uint8_t devId, uint32_t count, uint16_t buffer[], bool keepSel);
sr_t PlatformSpi_read32(uint8_t devId, uint32_t count, uint32_t buffer[], bool keepSel);
sr_t PlatformSpi_transfer8(uint8_t devId, uint32_t count, const uint8_t bufWrite[], uint8_t bufRead[], bool keepSel);
sr_t PlatformSpi_transfer16(uint8_t devId, uint32_t count, const uint16_t bufWrite[], uint16_t bufRead[], bool keepSel);
sr_t PlatformSpi_transfer32(uint8_t devId, uint32_t count, const uint32_t bufWrite[], uint32_t bufRead[], bool keepSel);

sr_t PlatformSpi_writeQspi8(uint8_t devId, uint32_t count, const uint8_t buffer[], bool keepSel);
sr_t PlatformSpi_readQspi8(uint8_t devId, uint32_t count, uint8_t buffer[], bool keepSel);

/** Use time constants in AurixTime.h for the timeout values */
sr_t PlatformSpi_setTimeout(uint8_t devId, uint32_t timeout);

/**
 * \brief This function asynchronously reads captured 12bit sample data from the BGT60TRxx chip into 16bit buffer.
 *
 * This function reads the sample data from the memory integrated in the
 * BGT60TRxx chip with a SPI burst transfer.

 * This function allows to specify a callback function and associated data which will be called once the
 * transfer is completed.
 *
 * The whole burst transfer is performed in a single SPI frame, so the chip
 * select signal (CS) goes LOW at the beginning and stays LOW until the end of
 * the burst transfer.
 *
 * \param         devId             SPI device Id
 * \param[out]    bufRead           A pointer to a buffer where received data
 * \param[in]     count             The number of words to be transferred.
 * \param[in]     cb                Function to call once the transfer
 *                                  has finished.
 * \param[in]     arg               Pointer to pass to the callback
 *                                        function.
 *
 */
sr_t PlatformSpi_readBurstAsync12(uint8_t devId, uint16_t *bufRead, uint32_t count, void (*cb)(void *), void *arg);

/**
 * \brief This function asynchronously reads captured sample data from the BGT60TRxx chip.
 *  The transactions are performed byte-wise resulting in packed data.
 *
 * This function reads the sample data from the memory integrated in the
 * BGT60TRxx chip with a SPI burst transfer.

 * This function allows to specify a callback function and associated data which will be called once the
 * transfer is completed.
 *
 * The whole burst transfer is performed in a single SPI frame, so the chip
 * select signal (CS) goes LOW at the beginning and stays LOW until the end of
 * the burst transfer.
 *
 * \param         devId             SPI device Id
 * \param[out]    bufRead           A pointer to a buffer where received data
 * \param[in]     count             The number of words to be transferred.
 * \param[in]     cb                Function to call once the transfer
 *                                  has finished.
 * \param[in]     arg               Pointer to pass to the callback
 *                                        function.
 *
 */
sr_t PlatformSpi_readBurstAsync8(uint8_t devId, uint8_t *bufRead, uint32_t count, void (*cb)(void *), void *arg);


#endif /* PLATFORM_SPI_H */

/** @} */
