@page Firmware_commands_8191_B11 Firmware commands for fw version 3.0.0-qsr.rc2

CTRX provides inbuilt Firmware commands which can be accessed using functions described at @ref IfxRfe_FirmwareCommands.c.
Configuration and execution of the radar application is done using these high-level firmware commands.

The user can implement their own firmware command calls using the commands provided at @ref Firmware_command_calls and the description for each firmware command from the user manual. The iRFE driver provides variations of each firmware commands as wrapper functions described below.

@subsection Wrapper_functions Wrapper functions available for the Firmware commands
Each firmware command can be accessed using different wrapper functions. These wrappers provide flexibility to the user to call the firmware as per their usage.
The wrappers allow the user to call each firmware command using all of the available command call methods described in @ref Firmware_command_calls. The Table below describes the types of wrappers available for each Firmware command and example of the wrapper function for firmware command 'Wait'.

For a firmware command 'xxxxxxx' there are the following wrapper commands available:
| Wrapper function type | Wrapper syntax | Wrapper function Description |Example Wrapper function in iRFE |
| :-------- |:----------- | :------------ | :-------- |
| Basic version (blocking) |***IfxRfe_xxxxxxx*** | Wrapper for firmware commands using using execute directly with parameters as a Structure| @ref IfxRfe_wait |
| Expanded version (blocking) |***IfxRfe_xxxxxxx_exp*** | Wrapper for firmware commands using using execute directly with expanded parameters| @ref IfxRfe_wait_exp |
| Start version (non-blocking) |***IfxRfe_xxxxxxx_start***| Asynchronous start wrapper for firmware commands using using execute directly | @ref IfxRfe_wait_start |
| Finish version (non-blocking) |***IfxRfe_xxxxxxx_finish*** | Asynchronous finish wrapper for firmware commands using using execute directly | @ref IfxRfe_wait_finish |
| Configure handle (new) |***IfxRfe_xxxxxxx_config*** | configure handle (new) wrapper for firmware commands| @ref IfxRfe_wait_config |
| Configure handle (update)|***IfxRfe_xxxxxxx_configUpdate***| configure handle (update) wrapper for firmware commands| @ref IfxRfe_wait_configUpdate |
| Execute handle |***IfxRfe_xxxxxxx_exec*** | execute handle & deserialize response for firmware commands| @ref IfxRfe_wait_exec |
| Configure and execute handle (new) |***IfxRfe_xxxxxxx_execConfig*** | configure & execute handle (new) wrapper for firmware commands| @ref IfxRfe_wait_execConfig |
| Configure and execute handle (update) |***IfxRfe_xxxxxxx_execConfigUpdate*** | configure & execute handle (update) wrapper for firmware commands| @ref IfxRfe_wait_execConfigUpdate |


All firmware commands described in the CTRX user manual are available through their wrappers in the iRFE Driver. The below table provides a list of the available firmware commands, example of a firmware command wrapper in iRFE and brief description of the firmware command.

@note The documentation of the driver only gives brief introduction to the Firmware commands for the CTRX. For detailed information please refer to the CTRX User Manual.

| Firmware Commands | Example Wrapper function in iRFE | Command Description |
| :-------------- | :------------ | :------- |
| ***Abort_Ramp_Scenario*** | @ref IfxRfe_abortRampScenario_exp | Abort the Sequencer. |
| ***Calculate_Memory_CRC*** | @ref IfxRfe_calculateMemoryCrc_exp | Calculate CRC of Memory Range. |
| ***Clear_Results*** | @ref IfxRfe_clearResults_exp | Clear requested result areas. |
| ***Configure_DIOs*** | @ref IfxRfe_configureDios_exp | Configure Digital IO Settings |
| ***Configure_DMUX*** | @ref IfxRfe_configureDmux_exp | Configure DMUX pins signal routing and direction. |
| ***Configure_MMIC_Clock*** | @ref IfxRfe_configureMmicClock_exp | Configure MMIC Clocks. |
| ***Configure_RF_Frequency*** | @ref IfxRfe_configureRfFrequency_exp | Change the DPLL's target frequency. It does not turn on the PAs automatically. |
| ***Configure_RX*** | @ref IfxRfe_configureRx_exp | Configure the receivers. |
| ***Configure_Ramp_Scenario*** | @ref IfxRfe_configureRampScenario_exp | Configure the sequencer for a ramp scenario based on the sequencer setup structure stored in memory. |
| ***Configure_TX_Power*** | @ref IfxRfe_configureTxPower_exp | Store up to 4 power levels along with power configuration timing for each transmitter channel. |
| ***Configure_VCID*** | @ref IfxRfe_configureVcid_exp | Configures virtual channes identifiers in the Low_Power and Operation states. |
| ***Download_FW_Functions*** | @ref IfxRfe_downloadFwFunctions_exp | Download a given block of RAM Firmware |
| ***Execute_Calibration*** | @ref IfxRfe_executeCalibration_exp | Performs the warm-up calibration of the MMIC |
| ***Execute_Monitoring*** | @ref IfxRfe_executeMonitoring_exp | Execute the monitoring routines |
| ***Execute_Self_Test*** | @ref IfxRfe_executeSelfTest_exp | Execute the selected self tests. |
| ***Exit_Pre_Operation*** | @ref IfxRfe_exitPreOperation_exp | Exit of pre_operation state and go to Low power state |
| ***Finish_FW_Download*** | @ref IfxRfe_finishFwDownload | Finalize the firmware download and execute an application reset (no return if successful, since it will execute a reset). |
| ***Finish_Ramp_Scenario*** | @ref IfxRfe_finishRampScenario | Wait for sequencer to complete the sequencer program and, after this, drive the device into the Operation state. |
| ***Get_RX_TF_Parameters*** | @ref IfxRfe_getRxTfParameters_exp | Return the RX transfer function parameters. |
| ***Get_Results*** | @ref IfxRfe_getResults_exp | Read a stored result from a given result area. |
| ***Get_Status*** | @ref IfxRfe_getStatus | Return the MMC's current status |
| ***Get_Temperature*** | @ref IfxRfe_getTemperature | Read the temperature from all the DTSs. |
| ***Get_Version*** | @ref IfxRfe_getVersion | Return the MMIC version information. |
| ***Goto_Low_Power*** | @ref IfxRfe_gotoLowPower | Go to Low Power State |
| ***Goto_Operation*** | @ref IfxRfe_gotoOperation | Go to Operation State |
| ***Handle_Error*** | @ref IfxRfe_handleError_exp | Read and optionally clear error flags. |
| ***Handle_Reset_Reason*** | @ref IfxRfe_handleResetReason_exp | Read or clear reset reason (previous error status). |
| ***Initialize*** | @ref IfxRfe_initialize_exp | Change of initialization settings |
| ***Measure_RX*** | @ref IfxRfe_measureRx_exp | Execute RX specific measurement routines. |
| ***Measure_TX*** | @ref IfxRfe_measureTx_exp | Measure the transmitted power (and possibly phase) of the Transmitter channels. |
| ***Read_Memory*** | @ref IfxRfe_readMemory_exp | Reads data words from a set of addresses. |
| ***Run_Batch*** | @ref IfxRfe_runBatch_exp | Execute a series of command handles. |
| ***Set_Clock_Output*** | @ref IfxRfe_setClockOutput_exp | Enable/Disable clock output. |
| ***Set_MMIC_Clock*** | @ref IfxRfe_setMmicClock_exp | Enable the clock input. |
| ***Set_Signals*** | @ref IfxRfe_setSignals_exp | Set General Purpose Signals. |
| ***Set_TX_Output*** | @ref IfxRfe_setTxOutput_exp | Enable or disable transmitter to transmit the RF signal. |
| ***Start_Ramp_Scenario*** | @ref IfxRfe_startRampScenario | Start a preconfigured ramp scenario |
| ***Trigger_LBIST*** | @ref IfxRfe_triggerLbist | Configure and trigger the LBIST  (no return if LBIST is triggered, since it will execute a reset). |
| ***Wait*** | @ref IfxRfe_wait_exp | Instructs firmware to wait for a specified time. |
| ***Write_Memory*** | @ref IfxRfe_writeMemory_exp | Write memory range of a specified region after verifying passkey as needed |

@note Some commands have optional fields in the request. For each of these optional fields, there is an extra boolean parameter in the parameter list, or the parameter structure (see @ref IfxRfe_CtrxCommandParams.h). Its name is in the format ***xxxxxx_present***, where xxxxxx is the name of the optional field. Use this flag to indicate for the iRFE, whether to serialize the optional field or not. Please refer to the CTRX User Manual to find out whether a field is optional, and when to use it.

@note Some commands have optional fields in the response. These fields are filled in by the CTRX, depending on the value in some other fields (please refer to the CTRX User Manual for the exact usage of these fields). For commands with optional response fields, the iRFE has an extra boolean return parameter (see @ref IfxRfe_CtrxCommandParams.h) with the name of ***optional_fields_available***. This field indicates, whether all of the optional fields are available in the response, or none of them. Since iRFE has no way to decide, which optional field is available in the SPI stream, either all of them have to be available, or none of them, otherwise the iRFE will return with an error. For commands with optional fields in the response, the array response values are also considered as optional.
