@page Convenience_functions Convenience functions

The driver also provides convenience functions for common actions required to setup the radar operation. These functions reduce the overhead for the user to setup the MMIC for the required application such as initializing the MMIC, downloading the RAM firmware, safely configuring the DMUX pins and Loading sequencer data. These functions are described in @ref IfxRfe.c

- @ref IfxRfe_init : This function initializes the iRFE middleware. It initializes the log interface. It initializes the SPI wrapper to set the spi driver device id. It initializes the GPIO wrapper to set the pin ids. It initializes the time wrapper and sets the flag for iRFE initialized.


- @ref IfxRfe_loadRamFw : This is a convenience function which downloads the RAM firmware. It takes ram FW Spi frame, number of frames and reset flag as arguments. Checks the arguments for missing parameters. Then downloads a sequence of ram firmware spi frames based on the number of frames and triggers a soft reset if the flag is set.


- @ref IfxRfe_ctrxInit : This convenience function does all the steps to Initialize the CTRX. It starts by configuring the SPI clock and downloading  the firmware in steps as described in the user manual. It then initializes the clock by setting the clock input, setting the SPI clock to 25MHz and setting the clock output. It then calls the initialize() firmware command and triggers a soft reset if configured. It then checks the version and transitions the CTRX to low power state by calling exit_pre_operation() command. After this it also checks the OK pin state to be high and returns an error code if this fails.


- @ref IfxRfe_calculateNmod : This is a convenience function to calculate the NMOD value for the @ref IfxRfe_fwCall_configureRfFrequency function based on the formula described in the user manual.


- @ref IfxRfe_calculateNcw : This is a convenience function to calculate the NCW value for the @ref IfxRfe_fwCall_configureRfFrequency function based on the formula described in the user manual.


- @ref IfxRfe_calculateRampBW : This is a convenience function to calculate the Ramp Bandwidth value for the @ref IfxRfe_fwCall_configureRfFrequency function based on the formula described in the user manual.


- @ref IfxRfe_loadSequencerData : This is a convenience function which loads sequencer setup data into CTRX Sequencer Memory from the start address argument. It utilizes the @ref IfxRfe_writeSequencerMemory to write the sequencer data words to the sequencer memory.


- @ref IfxRfe_writeSequencerMemory : This is a convenience function which writes a piece of the sequencer memory at the user defined address through SPI. It utilizes the write_memory() firmware command and writes the data words which are provided as argument. The start address and length of the data is also provided as arguments.


- @ref IfxRfe_readSequencerMemory : This is a convenience function which reads a piece of the sequencer memory from user defined address through SPI. It utilizes the read_memory() firmware command and reads the data words from the start address till length of the data provided as arguments.


- @ref IfxRfe_safeConfigureDmux : It is a convenience function to safely configure the DMUX pins by checking the configurations against each other first. The function checks if the Aurix DMUX configuration is compatible with CTRX DMUX configuration to be applied. It checks if the DMUX on both sides is not set to input or output at the same time to avoid signal contention. In case of mismatch it returns an error code 'IFXRFE_E_INVALID_PARAMETER' along with an error log message such as "DMUX1 is set to output on both Aurix and CTRX side".
After the checks, the Aurix inputs are configured first, then all the CTRX signals (single firmware command), and the Aurix outputs at the end.
