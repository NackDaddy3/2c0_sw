#ifndef __SPI_H__
#define __SPI_H__
#include "PlatformSpiDefinition.h"

#define SPI_CH_MMIC_A   0

////////////////////////////////////////////////////////////////////////////////
extern const PlatformSpiDefinition_Master boardSpiMasterDefinition[];
extern const PlatformSpiDefinition_Device boardSpiDeviceDefinition[];

#endif
