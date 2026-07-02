#ifndef HW_DEFINITIONS_CARKIT2C0_GMSL_H
#define HW_DEFINITIONS_CARKIT2C0_GMSL_H

#include <stdint.h>

#define GPIO_CHIP_0_ID 0
#define GPIO_CHIP_1_ID 1

// gpiochip 0 registered GPIOs 348 - 511
// gpiochip 1 registered GPIOs 316 - 347
// to get the offset for gpioXXX simply calculate XXX - lower base number
// e.g. gpio444 is on gpiochip0 so calculate 444 - 348 = 96

#define GPIO_OFFSET_RES_N_A 93   // PP.01 (gpio441)
#define GPIO_OFFSET_RES_N_B 141  // PAC.03 (gpio489)
#define GPIO_OFFSET_RFT_A   10   // PBB.02 (gpio236)
#define GPIO_OFFSET_RFT_B   142  // PAC.04 (gpio490)
#define GPIO_OFFSET_OK      4    // PAA.04 (gpio320)
#define GPIO_OFFSET_DMUX1_A 7    // PAA.07 (gpio323)
#define GPIO_OFFSET_SPI_RO  15   // PCC.03 (gpio331)
#define GPIO_OFFSET_SPI_BNE 140  // PAC.02 (gpio488)

#define GPIO_ID_RES_N_A (((GPIO_CHIP_0_ID & 0xFF) << 8) | (GPIO_OFFSET_RES_N_A & 0xFF))
#define GPIO_ID_RES_N_B (((GPIO_CHIP_0_ID & 0xFF) << 8) | (GPIO_OFFSET_RES_N_B & 0xFF))
#define GPIO_ID_RFT_A   (((GPIO_CHIP_1_ID & 0xFF) << 8) | (GPIO_OFFSET_RFT_A & 0xFF))
#define GPIO_ID_RFT_B   (((GPIO_CHIP_0_ID & 0xFF) << 8) | (GPIO_OFFSET_RFT_B & 0xFF))
#define GPIO_ID_OK      (((GPIO_CHIP_1_ID & 0xFF) << 8) | (GPIO_OFFSET_OK & 0xFF))
#define GPIO_ID_DMUX1_A (((GPIO_CHIP_1_ID & 0xFF) << 8) | (GPIO_OFFSET_DMUX1_A & 0xFF))
#define GPIO_ID_SPI_RO  (((GPIO_CHIP_1_ID & 0xFF) << 8) | (GPIO_OFFSET_SPI_RO & 0xFF))
#define GPIO_ID_SPI_BNE (((GPIO_CHIP_0_ID & 0xFF) << 8) | (GPIO_OFFSET_SPI_BNE & 0xFF))


//AG: from command param enums part.
//AG: what is the GPIO that is set going to though?

// /// \brief Configure DMUX config mask field
// typedef enum
// {
//     ConfigMask_DMUX1 = (1 << 0),
//     ConfigMask_DMUX2 = (1 << 1)
// } IfxRfe_configureDmux_configMask_t;

// /// \brief Configure DMUX alternate signal field
// typedef enum
// {
//     AltSignal_GeneralPurpose1    = 0x6D,  ///> General purpose signal 1
//     AltSignal_GeneralPurpose2    = 0x6E,  ///> General purpose signal 2
//     AltSignal_GeneralPurpose3    = 0x6F,  ///> General purpose signal 3
//     AltSignal_RxPayloadGateLevel = 0xA0,  ///> Sequencer RX_PAYLOAD_GATE level
//     AltSignal_RxPayloadGatePulse = 0xA1,  ///> Sequencer RX_PAYLOAD_GATE pulse
//     AltSignal_DmuxALevel         = 0xE9,  ///> Sequencer DMUX A level
//     AltSignal_DmuxAPulse         = 0xEA,  ///> Sequencer DMUX A pulse
//     AltSignal_DmuxBLevel         = 0xEB,  ///> Sequencer DMUX B level
//     AltSignal_DmuxBPulse         = 0xEC,  ///> Sequencer DMUX B pulse
//     AltSignal_DmuxCLevel         = 0xED,  ///> Sequencer DMUX C level
//     AltSignal_DmuxCPulse         = 0xEE,  ///> Sequencer DMUX C pulse
//     AltSignal_DmuxDLevel         = 0xEF,  ///> Sequencer DMUX D level
//     AltSignal_DmuxDPulse         = 0xF0   ///> Sequencer DMUX D pulse
// } IfxRfe_configureDmux_altSignal_t;

// /// \brief Configure DMUX pin direction field
// typedef enum
// {
//     DmuxDir_in  = 0,  ///> Input
//     DmuxDir_out = 1   ///> Output
// } IfxRfe_configureDmux_dir_t;




#endif  // HW_DEFINITIONS_CARKIT2C0_GMSL_H