#ifndef __MAX929X_H__
#define __MAX929X_H__

#define AR0239_TABLE_END 0xffff
#define MAX929X_DELAY  0x00
struct max929x_reg {
	u16 slave_addr;
	u16 reg;
	u16 val;
};


static struct max929x_reg max9296_LINKA_Dser_Ser_init[] = {
/*
# This script is validated on: 
# MAX9295D
# MAX9296B
# Please refer to the Errata sheet for each device.
# ---------------------------------------------------------------------------------
*/
//  
// CSIConfigurationTool
//  
// GMSL-A / Serializer: MAX9295D (Pixel Mode) / Mode: 2x4, A&B / Device Address: 0x80 / Multiple-VC Case: Single VC / Pipe Sharing: Separate Pipes
// PipeX:
// Input Stream: VC0 RAW12 PortA (D-PHY) (Doubled)
// PipeY:
// Input Stream: VC0 RAW12 PortB (D-PHY) (Doubled)

// Deserializer: MAX9296B / Mode: 2 (1x4) / Device Address: 0x90
// PipeX:
// GMSL-A Input Stream: VC0 RAW12 PortA - Output Stream: VC0 RAW12 PortA (D-PHY)
// PipeY:
// GMSL-A Input Stream: VC0 RAW12 PortB - Output Stream: VC0 RAW12 PortB (D-PHY)

	{0x90,0x0313,0x00}, //  (CSI_OUT_EN): CSI output disabled
	// Link Initialization for Deserializer
	{0x90,0x0010,0x31}, //  (AUTO_LINK): Enabled | (Default)  (LINK_CFG): 0x1 |  (RESET_ONESHOT): Activated
	{MAX929X_DELAY, 0x0000, 0x78}, // 120 msec delay
	// Video Transmit Configuration for Serializer(s)
	{0x80,0x0002,0x43}, // DEV : REG2 | VID_TX_EN_X (VID_TX_EN_X): Disabled | (Default) VID_TX_EN_Y (VID_TX_EN_Y): Disabled

	{0x90,0x0003,0x00},
	{0x90,0x0170,0x08},
	{0x90,0x0172,0x07},
	{0x90,0x0176,0x03},
	{0x80,0x0173,0x1E},
	{0x80,0x0174,0x1E},
	{0x80,0x0175,0x1E},
	{0x80,0x0176,0x0C},
	{0x80,0x0172,0x0C},
	{0x80,0x0170,0x0A},
	{MAX929X_DELAY, 0x0000, 0x78}, // 120 msec delay

	//  
	// INSTRUCTIONS FOR GMSL-A SERIALIZER MAX9295D
	//  
	// MIPI D-PHY Configuration
	{0x80,0x0330,0x06}, // MIPI_RX : MIPI_RX0 | phy_config (Port Configuration): 2x4, A&B
	{0x80,0x0331,0x33}, // MIPI_RX : MIPI_RX1 | (Default) ctrl0_num_lanes (Port A - Lane Count): 4
	{0x80,0x0332,0x4E}, // MIPI_RX : MIPI_RX2 | (Default) Port A lane Mapping (Lane Map - PHY0 D0): Lane 2 | (Default) Port A lane Mapping (Lane Map - PHY0 D1): Lane 3 | phy1_lane_map (Lane Map - PHY1 D0): Lane 0 | phy1_lane_map (Lane Map - PHY1 D1): Lane 1
	{0x80,0x0334,0x00}, // MIPI_RX : MIPI_RX4 | (Default) phy0_pol_map (Polarity - PHY0 Lane 0): Normal | (Default) phy0_pol_map (Polarity - PHY0 Lane 1): Normal | (Default) phy1_pol_map (Polarity - PHY1 Lane 0): Normal | (Default) phy1_pol_map (Polarity - PHY1 Lane 1): Normal | (Default) phy1_pol_map (Polarity - PHY1 Clock Lane): Normal
	{0x80,0x0331,0x33}, // MIPI_RX : MIPI_RX1 | (Default) ctrl1_num_lanes (Port B - Lane Count): 4
	{0x80,0x0333,0xE4}, // MIPI_RX : MIPI_RX3 | (Default) phy2_lane_map (Lane Map - PHY2 D0): Lane 0 | (Default) phy2_lane_map (Lane Map - PHY2 D1): Lane 1 | (Default) phy3_lane_map (Lane Map - PHY3 D0): Lane 2 | (Default) phy3_lane_map (Lane Map - PHY3 D1): Lane 3
	{0x80,0x0335,0x00}, // MIPI_RX : MIPI_RX5 | (Default) phy2_pol_map (Polarity - PHY2 Lane 0): Normal | (Default) phy2_pol_map (Polarity - PHY2 Lane 1): Normal | (Default) phy3_pol_map (Polarity - PHY3 Lane 0): Normal | (Default) phy3_pol_map (Polarity - PHY3 Lane 1): Normal | (Default) phy2_pol_map (Polarity - PHY2 Clock Lane): Normal
	// Controller to Pipe Mapping Configuration
	{0x80,0x0308,0x7E}, // FRONTTOP : FRONTTOP_0 | (Default) CLK_SELX (CLK_SELX): Port A | CLK_SELY (CLK_SELY): Port B | (Default) START_PORTA (START_PORTA): Enabled
	{0x80,0x0311,0x51}, // FRONTTOP : FRONTTOP_9 | (Default) START_PORTAX (START_PORTAX): Start Video | (Default) START_PORTAY (START_PORTAY): Not Started | START_PORTAZ (START_PORTAZ): Not Started | (Default) START_PORTAU (START_PORTAU): Not Started
	{0x80,0x0308,0x7E}, // FRONTTOP : FRONTTOP_0 | (Default) START_PORTB (START_PORTB): Enabled
	{0x80,0x0311,0x21}, // FRONTTOP : FRONTTOP_9 | START_PORTBX (START_PORTBX): Not Started | START_PORTBY (START_PORTBY): Start Video | START_PORTBZ (START_PORTBZ): Not Started | (Default) START_PORTBU (START_PORTBU): Not Started
	{0x80,0x0314,0x6C}, // FRONTTOP : FRONTTOP_12 | mem_dt1_selx (mem_dt1_selx): 0x6C
	{0x80,0x0316,0x6C}, // FRONTTOP : FRONTTOP_14 | mem_dt1_sely (mem_dt1_sely): 0x6C
	// Double Mode Configuration
	{0x80,0x0313,0x10}, // FRONTTOP : FRONTTOP_11 | bpp12dblx (bpp12dblx): Send 12-bit pixels as 24-bit
	{0x80,0x031C,0x38}, // FRONTTOP : FRONTTOP_20 | (Default) soft_bppx (soft_bppx): 0x18 | soft_bppx_en (soft_bppx_en): Software override enabled
	{0x80,0x0313,0x30}, // FRONTTOP : FRONTTOP_11 | bpp12dbly (bpp12dbly): Send 12-bit pixels as 24-bit
	{0x80,0x031D,0x38}, // FRONTTOP : FRONTTOP_21 | (Default) soft_bppy (soft_bppy): 0x18 | soft_bppy_en (soft_bppy_en): Software override enabled
	// Pipe Configuration
	{0x80,0x0053,0x10}, // CFGV__VIDEO_X : TX3 | (Default) TX_STR_SEL (TX_STR_SEL Pipe X): 0x0
	{0x80,0x0057,0x11}, // CFGV__VIDEO_Y : TX3 | (Default) TX_STR_SEL (TX_STR_SEL Pipe Y): 0x1
	//  
	// INSTRUCTIONS FOR DESERIALIZER MAX9296B
	//  
	// Video Pipes And Routing Configuration
	{0x90,0x0050,0x00}, // (Default)  (STR_SELX): 0x0
	{0x90,0x0051,0x01}, // (Default)  (STR_SELY): 0x1
	// Pipe to Controller Mapping Configuration
	{0x90,0x040B,0x07}, //  (MAP_EN_L Pipe X): 0x7
	{0x90,0x040C,0x00}, // (Default)  (MAP_EN_H Pipe X): 0x0
	{0x90,0x040D,0x2C}, //  (MAP_SRC_0 Pipe X DT): 0x2C | (Default)  (MAP_SRC_0 Pipe X VC): 0x0
	{0x90,0x040E,0x2C}, //  (MAP_DST_0 Pipe X DT): 0x2C | (Default)  (MAP_DST_0 Pipe X VC): 0x0
	{0x90,0x040F,0x00}, // (Default)  (MAP_SRC_1 Pipe X DT): 0x0 | (Default)  (MAP_SRC_1 Pipe X VC): 0x0
	{0x90,0x0410,0x00}, // (Default)  (MAP_DST_1 Pipe X DT): 0x0 | (Default)  (MAP_DST_1 Pipe X VC): 0x0
	{0x90,0x0411,0x01}, //  (MAP_SRC_2 Pipe X DT): 0x1 | (Default)  (MAP_SRC_2 Pipe X VC): 0x0
	{0x90,0x0412,0x01}, //  (MAP_DST_2 Pipe X DT): 0x1 | (Default)  (MAP_DST_2 Pipe X VC): 0x0
	{0x90,0x042D,0x15}, //  (MAP_DPHY_DST_0 Pipe X): 0x1 |  (MAP_DPHY_DST_1 Pipe X): 0x1 |  (MAP_DPHY_DST_2 Pipe X): 0x1
	{0x90,0x044B,0x07}, //  (MAP_EN_L Pipe Y): 0x7
	{0x90,0x044C,0x00}, // (Default)  (MAP_EN_H Pipe Y): 0x0
	{0x90,0x044D,0x2C}, //  (MAP_SRC_0 Pipe Y DT): 0x2C | (Default)  (MAP_SRC_0 Pipe Y VC): 0x0
	{0x90,0x044E,0x2C}, //  (MAP_DST_0 Pipe Y DT): 0x2C | (Default)  (MAP_DST_0 Pipe Y VC): 0x0
	{0x90,0x044F,0x00}, // (Default)  (MAP_SRC_1 Pipe Y DT): 0x0 | (Default)  (MAP_SRC_1 Pipe Y VC): 0x0
	{0x90,0x0450,0x00}, // (Default)  (MAP_DST_1 Pipe Y DT): 0x0 | (Default)  (MAP_DST_1 Pipe Y VC): 0x0
	{0x90,0x0451,0x01}, //  (MAP_SRC_2 Pipe Y DT): 0x1 | (Default)  (MAP_SRC_2 Pipe Y VC): 0x0
	{0x90,0x0452,0x01}, //  (MAP_DST_2 Pipe Y DT): 0x1 | (Default)  (MAP_DST_2 Pipe Y VC): 0x0
	{0x90,0x046D,0x2A}, //  (MAP_DPHY_DST_0 Pipe Y): 0x2 |  (MAP_DPHY_DST_1 Pipe Y): 0x2 |  (MAP_DPHY_DST_2 Pipe Y): 0x2
	// Double Mode Configuration
	{0x90,0x0473,0x01}, //  (ALT_MEM_MAP12 CTRL1): Alternate memory map enabled
	{0x90,0x04B3,0x01}, //  (ALT_MEM_MAP12 CTRL2): Alternate memory map enabled
	// MIPI D-PHY Configuration
	{0x90,0x0330,0x04}, // (Default)  (Port Configuration): 2 (1x4)
	{0x90,0x044A,0xD0}, // (Default)  (Port A - Lane Count): 4
	{0x90,0x0333,0x4E}, // (Default)  (Lane Map - PHY0 D0): Lane 2 | (Default)  (Lane Map - PHY0 D1): Lane 3 | (Default)  (Lane Map - PHY1 D0): Lane 0 | (Default)  (Lane Map - PHY1 D1): Lane 1
	{0x90,0x0335,0x00}, // (Default)  (Polarity - PHY0 Lane 0): Normal | (Default)  (Polarity - PHY0 Lane 1): Normal | (Default)  (Polarity - PHY1 Lane 0): Normal | (Default)  (Polarity - PHY1 Lane 1): Normal | (Default)  (Polarity - PHY1 Clock Lane): Normal
	{0x90,0x1D00,0xF4}, //  (config_soft_rst_n - PHY1): 0x0
	// This is to set predefined (coarse) CSI output frequency
	// CSI Phy 1 is 1200 Mbps/lane.
	{0x90,0x0332,0x00}, //  (CSI_OUT_EN): CSI output Disabled
	{0x90,0x0320,0x2C},
	{0x90,0x1D00,0xF5}, //  (config_soft_rst_n - PHY1): 0x1
	{0x90,0x048A,0xD0}, // (Default)  (Port B - Lane Count): 4
	{0x90,0x0334,0xE4}, // (Default)  (Lane Map - PHY2 D0): Lane 0 | (Default)  (Lane Map - PHY2 D1): Lane 1 | (Default)  (Lane Map - PHY3 D0): Lane 2 | (Default)  (Lane Map - PHY3 D1): Lane 3
	{0x90,0x0336,0x00}, // (Default)  (Polarity - PHY2 Lane 0): Normal | (Default)  (Polarity - PHY2 Lane 1): Normal | (Default)  (Polarity - PHY3 Lane 0): Normal | (Default)  (Polarity - PHY3 Lane 1): Normal | (Default)  (Polarity - PHY2 Clock Lane): Normal
	{0x90,0x1E00,0xF4}, //  (config_soft_rst_n - PHY2): 0x0
	// This is to set predefined (coarse) CSI output frequency
	// CSI Phy 2 is 1200 Mbps/lane.
	{0x90,0x0323,0x2C},
	{0x90,0x1E00,0xF5}, //  (config_soft_rst_n - PHY2): 0x1
	{0x90,0x0313,0x02}, //  (CSI_OUT_EN): CSI output enabled
	// Video Transmit Configuration for Serializer(s)
	{0x80,0x0002,0x43}, // DEV : REG2 | VID_TX_EN_X (VID_TX_EN_X): Disabled | VID_TX_EN_Y (VID_TX_EN_Y): Disabled

	{0x90,0x0005,0x00},
	{0x80,0x0005,0x00},
	//AB OK DES8<-SER1
	{0x90,0x02B3,0x04},
	{0x90,0x02B4,0xA1},
	{0x90,0x02B5,0x41},
	{0x80,0x02D6,0x9B},
	{0x80,0x02D7,0xA1},
	{0x80,0x02D8,0x41},
	//A RFT DES3<-SER12
	{0x90,0x02B9,0x04},
	{0x90,0x02BA,0xA2},
	{0x90,0x02BB,0x42},
	{0x80,0x02E2,0x9B},
	{0x80,0x02E3,0xA2},
	{0x80,0x02E4,0x42},
	//B RFT DES4<-SER13
	{0x90,0x02BC,0x04},
	{0x90,0x02BD,0xA3},
	{0x90,0x02BE,0x43},
	{0x80,0x02E5,0x9B},
	{0x80,0x02E6,0xA3},
	{0x80,0x02E7,0x43},
	//A RESET DES8->SER9
	{0x90,0x02C8,0x9B},
	{0x90,0x02C9,0xA4},
	{0x90,0x02CA,0x44},
	{0x80,0x02D9,0x84},
	{0x80,0x02DA,0xA4},
	{0x80,0x02DB,0x44},
	//A DMUX1 DES9->SER3
	{0x90,0x02CB,0x9B},
	{0x90,0x02CC,0xA5},
	{0x90,0x02CD,0x45},
	{0x80,0x02C7,0x84},
	{0x80,0x02C8,0xA5},
	{0x80,0x02C9,0x45},
	//B RESET DES10->SER7
	{0x90,0x02CE,0x9B},
	{0x90,0x02CF,0xA6},
	{0x90,0x02D0,0x46},
	{0x80,0x02D3,0x84},
	{0x80,0x02D4,0xA6},
	{0x80,0x02D5,0x46},

	{0x90,0x0170,0x09},
	{0x80,0x0170,0x0B},

};


#endif

