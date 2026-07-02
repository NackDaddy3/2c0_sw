/*
 * File: PlatformGpio_IDs_JetsonAGX_CRK_Adapter_V10.h
 * Description: GPIO IDs for the "JetsonAGX_CRK_Adapter V10" adapter board
 * Project: MIMOrad
 * Created Date: Wednesday, 08 January 2025, 15:23
 * Author: Daniel Klepatsch (daniel.klepatsch@silicon-austria.com)
 * ---------------
 * Last Modified: Wednesday, 08 January 2025, 16:03
 * Modified By: Daniel Klepatsch
 * ---------------
 * Copyright (c) 2025 Silicon Austria Labs GmbH
 */

#ifndef PLATFORMGPIO_IDS_JETSONAGX_CRK_ADAPTER_V10_H
#define PLATFORMGPIO_IDS_JETSONAGX_CRK_ADAPTER_V10_H

#include "PlatformGpio.h"

// Pin Offsets

#define GPIO_OFFSET_SPI_SS_A    10
#define GPIO_OFFSET_SPI_SS_B    15
#define GPIO_OFFSET_SPI_SS_C    4
#define GPIO_OFFSET_SPI_SS_D    145

#define GPIO_OFFSET_OK_A        19
#define GPIO_OFFSET_RFT_A       20
#define GPIO_OFFSET_RES_N_A     49

#define GPIO_OFFSET_OK_B        141
#define GPIO_OFFSET_RFT_B       44
#define GPIO_OFFSET_RES_N_B     138

#define GPIO_OFFSET_OK_C        91
#define GPIO_OFFSET_RFT_C       84
#define GPIO_OFFSET_RES_N_C     46

#define GPIO_OFFSET_OK_D        142
#define GPIO_OFFSET_RFT_D       48
#define GPIO_OFFSET_RES_N_D     93

// Pin IDs

#define GPIO_ID_SPI_SS_A    (((GPIO_CHIP_1_ID & 0xFF) << 8) | (GPIO_OFFSET_SPI_SS_A & 0xFF))
#define GPIO_ID_SPI_SS_B    (((GPIO_CHIP_1_ID & 0xFF) << 8) | (GPIO_OFFSET_SPI_SS_B & 0xFF))
#define GPIO_ID_SPI_SS_C    (((GPIO_CHIP_1_ID & 0xFF) << 8) | (GPIO_OFFSET_SPI_SS_C & 0xFF))
#define GPIO_ID_SPI_SS_D    (((GPIO_CHIP_0_ID & 0xFF) << 8) | (GPIO_OFFSET_SPI_SS_D & 0xFF))

#define GPIO_ID_OK_A        (((GPIO_CHIP_1_ID & 0xFF) << 8) | (GPIO_OFFSET_OK_A & 0xFF))
#define GPIO_ID_RFT_A       (((GPIO_CHIP_1_ID & 0xFF) << 8) | (GPIO_OFFSET_RFT_A & 0xFF))
#define GPIO_ID_RES_N_A     (((GPIO_CHIP_0_ID & 0xFF) << 8) | (GPIO_OFFSET_RES_N_A & 0xFF))

#define GPIO_ID_OK_B        (((GPIO_CHIP_0_ID & 0xFF) << 8) | (GPIO_OFFSET_OK_B & 0xFF))
#define GPIO_ID_RFT_B       (((GPIO_CHIP_0_ID & 0xFF) << 8) | (GPIO_OFFSET_RFT_B & 0xFF))
#define GPIO_ID_RES_N_B     (((GPIO_CHIP_0_ID & 0xFF) << 8) | (GPIO_OFFSET_RES_N_B & 0xFF))

#define GPIO_ID_OK_C        (((GPIO_CHIP_0_ID & 0xFF) << 8) | (GPIO_OFFSET_OK_C & 0xFF))
#define GPIO_ID_RFT_C       (((GPIO_CHIP_0_ID & 0xFF) << 8) | (GPIO_OFFSET_RFT_C & 0xFF))
#define GPIO_ID_RES_N_C     (((GPIO_CHIP_0_ID & 0xFF) << 8) | (GPIO_OFFSET_RES_N_C & 0xFF))

#define GPIO_ID_OK_D        (((GPIO_CHIP_0_ID & 0xFF) << 8) | (GPIO_OFFSET_OK_D & 0xFF))
#define GPIO_ID_RFT_D       (((GPIO_CHIP_0_ID & 0xFF) << 8) | (GPIO_OFFSET_RFT_D & 0xFF))
#define GPIO_ID_RES_N_D     (((GPIO_CHIP_0_ID & 0xFF) << 8) | (GPIO_OFFSET_RES_N_D & 0xFF))

#endif // PLATFORMGPIO_IDS_JETSONAGX_CRK_ADAPTER_V10_H
