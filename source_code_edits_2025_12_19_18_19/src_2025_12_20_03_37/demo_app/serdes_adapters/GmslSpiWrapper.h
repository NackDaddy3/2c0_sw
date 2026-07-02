#ifndef GMSLSPIWRAPPER_H
#define GMSLSPIWRAPPER_H

#include "PlatformSpi.h"
#include <stdbool.h>
#include <stdint.h>

int GmslSpiWrapper_configure(uint8_t spiId, uint8_t flags, uint32_t speed);
int GmslSpiWrapper_write(uint8_t spiId, uint32_t count, const uint32_t buffer[], bool keepSel);
int GmslSpiWrapper_transfer(uint8_t spiId, uint32_t count, const uint32_t bufWrite[], uint32_t bufRead[],
                            bool keepSel);

/**
 * @brief Sets the wrapper configuration
 * 
 * @param devIds Array of device IDs for the SPI devices
 * @param devCnt Number of SPI devices
 * @param ro GPIO ID for the Read Only (RO) pin
 * @param bne GPIO ID for the Buffer Not Empty (BNE) pin
 * 
 * @return Status code indicating success or failure
 */
int GmslSpiWrapper_setConfig(uint8_t *devIds, uint8_t devCnt, uint16_t ro, uint16_t bne);

/**
 * @brief Deinitialize the SPI interface
 * 
 * Deasserts the CSs over GMSL (by sending 0xA6 command) and calls PlatformSpi_deinit().
 * 
 * @return void
 */
void GmslSpiWrapper_deinit();

#endif  // GMSLSPIWRAPPER_H