/**
 * \addtogroup      PlatformSpi
 * @{
 */
#ifndef PLATFORM_SPI_DEFINITION_H
#define PLATFORM_SPI_DEFINITION_H 1

#include "Platform_Types.h"
#include <_PinMap/IfxQspi_PinMap.h>


typedef struct
{
    IfxQspi_Sclk_Out *mclk;  ///< master clock I/O pin
    IfxQspi_Mtsr_Out *mosi;  ///< master MOSI output pin
    IfxQspi_Mrst_In *miso;   ///< master MISO input pin
    uint32 maxSpeed;         ///< SPI max baud rate
} PlatformSpiDefinition_Master;

typedef struct
{
    IfxQspi_Slso_Out *ssel;  ///< device slave select output pin
} PlatformSpiDefinition_Device;

#endif /* PLATFORM_SPI_DEFINITION_H */

/** @} */
