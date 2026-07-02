@page Low_level_commands Low level commands

Low level commands provide functionality on CTRX for the Housekeeping tasks, like triggering a device reset or a read-back of the last SPI response


### Available Low level commands with their driver interfaces


| SPI Commands | Function in iRFE | Command Description |
| :-------------- | :------------ | :------- |
|  ***Do_NOP*** |  @ref IfxRfe_doNOP | This low level command is used to read the last response or the reset response from the MMIC without triggering any new command processing. |
|  ***Trigger_Reset*** |  @ref IfxRfe_triggerReset | This low level command triggers a sof reset of the device and a transition into the SW_Startup state. |



@note The documentation of the driver only gives brief introduction to the SPI commands for the CTRX. For detailed information please refer to the CTRX User Manual.
