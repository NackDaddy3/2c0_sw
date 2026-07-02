#include "config.h"
#include "IfxRfe_CommandParamEnums.h"
#include <stdio.h>


void IrfeDemoConfigInit(IfxRfe_demoConfigParams_t *configParams, IfxRfe_deviceConfiguration_t config)
{
    // Ctrx Dmux config
    //So set whatever this mask is to BOTH dmux1 and dmux2 pins 
    configParams->ctrxDmux.config_mask = ConfigMask_DMUX1 | ConfigMask_DMUX2; //AG: enable both dmux?
    // configParams->ctrxDmux.dmux1_dir = configParams->ctrxDmux.dmux2_dir = DmuxDir_out; // AG: confifugre as an out
    //AG: where does the dmux2 go to then? 
        //AG: it is possible that this sets the demux1 only for BOTH devices, and dmux2 is unused
    //AG: Leave direction as out
    configParams->ctrxDmux.dmux1_alt_signal = configParams->ctrxDmux.dmux2_alt_signal = DmuxDir_out; // AG: confifugre as an out
    //AG: set to RX Payload gate pulse
    configParams->ctrxDmux.dmux1_dir = configParams->ctrxDmux.dmux2_dir = AltSignal_RxPayloadGatePulse; // AG: confifugre as an out
   
   
  
    
    //AG: original
    //Then set the pulse duration (extentension) for both to be the same. 
    configParams->ctrxDmux.dmux1_pulse_duration_ext = configParams->ctrxDmux.dmux2_pulse_duration_ext = 63;  // Pulse duration of DMUX1-3 (0: disabled, 3..63: (n+1)*5ns)

    //configure DIO-s on ctrx
    configParams->dios.config_mask_ext      = DioConfig_RFT | DioConfig_DMUX1 | DioConfig_DMUX2;
    //AG: changed because want RPG to be strongest
    // configParams->dios.dmux1_drive_strength = configParams->dios.dmux2_drive_strength = DriveStrength_Weak;
    configParams->dios.dmux1_drive_strength = configParams->dios.dmux2_drive_strength = DriveStrength_Strongest;
    configParams->dios.dmux1_driver_config = configParams->dios.dmux2_driver_config = DriverConfig_PushPull;
    configParams->dios.dmux1_pull_config = configParams->dios.dmux2_pull_config = PullConfig_NotSelected;

    configParams->dios.rft_drive_strength = DriveStrength_Strong;
    configParams->dios.rft_driver_config  = DriverConfig_PushPull;
    configParams->dios.rft_pull_config    = DriverConfig_NotSelected;

    //configure the tx power
    //parameters in plvlx_2, plvlx_1, plvlx_4, plvlx_3 order because of code generation
    configParams->txpwr.tx1_plvl_2              = 3 * 128;
    configParams->txpwr.tx1_plvl_1              = 10 * 128;
    configParams->txpwr.tx1_plvl_4              = 20 * 128;
    configParams->txpwr.tx1_plvl_3              = 10 * 128;
    configParams->txpwr.tx2_plvl_2              = 3 * 128;
    configParams->txpwr.tx2_plvl_1              = 10 * 128;
    configParams->txpwr.tx2_plvl_4              = 20 * 128;
    configParams->txpwr.tx2_plvl_3              = 10 * 128;
    configParams->txpwr.tx3_plvl_2              = 3 * 128;
    configParams->txpwr.tx3_plvl_1              = 10 * 128;
    configParams->txpwr.tx3_plvl_4              = 20 * 128;
    configParams->txpwr.tx3_plvl_3              = 10 * 128;
    configParams->txpwr.tx4_plvl_2              = 3 * 128;
    configParams->txpwr.tx4_plvl_1              = 10 * 128;
    configParams->txpwr.tx4_plvl_4              = 20 * 128;
    configParams->txpwr.tx4_plvl_3              = 10 * 128;
    configParams->txpwr.tx1_pa_slope_scale_fact = 1 * 256;
    configParams->txpwr.tx2_pa_slope_scale_fact = 1 * 256;
    configParams->txpwr.tx3_pa_slope_scale_fact = 1 * 256;
    configParams->txpwr.tx4_pa_slope_scale_fact = 1 * 256;

    //Configure Rx Frontend
    configParams->rxcfg.gain                 = GainSel_0dB;
    configParams->rxcfg.hp1                  = RxHp1_Cutoff_200kHz;
    configParams->rxcfg.hp2                  = RxHp2_Cutoff_120kHz;
    configParams->rxcfg.hp2_en               = false;
    configParams->rxcfg.data_width           = DataWidth_12bits;
    configParams->rxcfg.dec_sel              = DecSel_50MS;
    configParams->rxcfg.hsrif_csi2_data_rate = DataRate_600Mbitsps;
    configParams->rxcfg.hsrif_start_mode     = StartMode_Immediate;


    // RF Frequency for parameters from Coc Korea
    configParams->rfFreqs.f_static = 76.5;  //static frequency in MHz before ramp sequence starts
    configParams->rfFreqs.f_lock   = 77.0;  //Upper frequency of the RF modulation bandwidth in MHz
    configParams->rfFreqs.f_bw     = 0.99;  //RF modulation bandwidth in MHz

    configParams->rfFreqCfg.bc     = 1;
    configParams->rfFreqCfg.nmod   = 62914560U;   //IfxRfe_calculateNmod(configParams->rfFreqs.f_static, configParams->rfFreqs.f_lock);
    configParams->rfFreqCfg.ncw    = 272979280U;  // IfxRfe_calculateNcw(configParams->rfFreqs.f_lock); //;
    configParams->rfFreqCfg.rampbw = 4194304U;    // IfxRfe_calculateRampBW(configParams->rfFreqs.f_bw); //

    printf("\nnmod=%u, ncw=%u, rampbw=%u\n", configParams->rfFreqCfg.nmod, configParams->rfFreqCfg.ncw, configParams->rfFreqCfg.rampbw);

    //Ramp Scenario
    // configParams->rampScenario.config_addr = 0;  //Sequencer setup structure start address of sequencer program

    // Calibration Parameters
    if (config == PRIMARY)
        configParams->calibration.calib_sub_func_id = 0b001101100000000;
    else if (config == SECONDARY)
        configParams->calibration.calib_sub_func_id = 0b001101100000000;
    else if (config == STANDALONE)
        configParams->calibration.calib_sub_func_id = 0b001101100000000;  //0x1B07;  //enable calibration subfunctions


    configParams->calibration.tx_ch_pow_idx = 0xFFFF;  // enable power calibration for all power levels at all TX channels
    // configParams->calibration.detail_result     = 0;
    configParams->calibration.ref_temp_idx = 0;  //no reference temperature. Execute_Calibration() is based only on the LimitTemp. The user has to determine both the reported MMIC temperature and the temperature after the previous calibration.
    configParams->calibration.limit_temp   = 0;  //Calibration shall be called if |latest temperature - reference temperature| > LimitTemp (scaled in Q12.3 format), 0: calibrate regardless of current temperature.
}