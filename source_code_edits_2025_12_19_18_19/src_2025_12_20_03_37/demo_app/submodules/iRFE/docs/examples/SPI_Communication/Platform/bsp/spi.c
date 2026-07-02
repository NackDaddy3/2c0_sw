#include "spi.h"

////////////////////////////////////////////////////////////////////////////////
const PlatformSpiDefinition_Master boardSpiMasterDefinition[] = {
    {
        .mclk     = &IfxQspi1_SCLK_P10_2_OUT,
        .mosi     = &IfxQspi1_MTSR_P10_4_OUT,
        .miso     = &IfxQspi1_MRSTA_P10_1_IN,
        .maxSpeed = 25000000,
    }};

const PlatformSpiDefinition_Device boardSpiDeviceDefinition[] = {
    [SPI_CH_MMIC_A] = {
        .ssel = &IfxQspi1_SLSO0_P20_8_OUT,
    }};

//////////////////////////////////////////////////////////////////////////////////
