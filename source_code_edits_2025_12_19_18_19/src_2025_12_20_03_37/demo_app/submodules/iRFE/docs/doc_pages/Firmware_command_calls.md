@page Firmware_command_calls Firmware command calls

Firmware command calls provide functionality on CTRX for Configuration or execution of high-level firmware commands. This can be a single firmware command or a sequence of multiple firmware commands defined with batches. 

@section Available_FW_Commands Available Firmware command calls with their driver interfaces


| SPI Commands | Function in iRFE | Command Description |
| :-------------- | :------------ | :------- |
|  ***Execute_Directly*** |  @ref IfxRfe_fwCall_executeDirectly | This firmware command call triggers the execution of a firmware command using the parameters sent as part of the SPI command. |
|  ***Configure_Handle*** |  @ref IfxRfe_fwCall_configureHandle | This firmware command call validates and stores a set of parameters used for an intended firmware command and returns a handle of the stored configuration as response. |
|  ***Execute_Handle*** |  @ref IfxRfe_fwCall_executeHandle | This firmware command call triggers the execution of a firmware command with the already stored configuration using the associated handle. |
|  ***Configure_And_Execute_Handle*** |  @ref IfxRfe_fwCall_configureAndExecuteHandle | This firmware command call combines the two calls Configure_Handle() and Execute_Handle() into one command. |

@subsection Additional_FW_functions Additional Firmware function calls in iRFE

| SPI Command used | Function in iRFE | function Description |
| :-------------- | :------------ | :------- |
|  ***Execute_Directly*** |  @ref IfxRfe_fwCall_executeDirectly_send | Execute the Execute_Directly() firmware command, without waiting for it to finish. |
|  ***Execute_Directly*** |  @ref IfxRfe_fwCall_executeDirectly_read | Read back the result of the last Execute_Directly() firmware command. The function waits for the RFT pin blockingly to ensure the sent command is finished |

@note The documentation of the driver only gives brief introduction to the SPI commands for the CTRX. For detailed information please refer to the CTRX User Manual.
 
@section Methods_of_using_FWcmd_Calls Programming methods using Firmware command calls

@subsection FW_cmds_directly Executing the firmware commands directly and blocking
In this method the user can call firmware commands using Execute_Directly(). The firmware command call in this method is blocking in nature, i.e. the controller waits for the command to be executed. The user can typically call the firmware commands using the wrappers provided by the iRFE or use the firmware command calls to call the firmware commands themselves. Using this method of firmware command calls two types of wrappers can be typically used from iRFE such as @ref IfxRfe_wait and @ref IfxRfe_wait_exp.

Option 1: Firmware command call by the user without using iRFE wrapper
```c
    // this involves using the firmware command calls and providing the arguments in the correct format required by the firmware command

    IfxRfe_ctrxRequest_t cmd;
    IfxRfe_ctrxResponse_t rsp;

    // configuring the payload parameters and payload length for the firmware command
    cmd.buffer[1] = wait_time;

    cmd.payloadLen = 2;

    // Call the firmware command using execute directly
    IfxRfe_fwCall_executeDirectly(IFXRFE_FW_COMMAND_WAIT, &cmd, &rsp)
```

Option 2: FW command call using iRFE wrapper (basic version)
```c
    // FW command call using using execute directly with parameters as a Structure   

    Error_return = IfxRfe_wait(_configParams.wait);
```

Option 3: FW command call using iRFE wrapper (expanded version)
```c
    // FW command call using using execute directly with expanded parameters  

    Error_return = IfxRfe_wait_exp(_configParams.wait.wait_time);
```


@subsection FW_cmds_non_blocking Executing the firmware commands using non blocking calls 
In this method the user can call firmware commands using @ref IfxRfe_fwCall_executeDirectly_send and @ref IfxRfe_fwCall_executeDirectly_read. The firmware command calls are non-blocking in nature, i.e. the controller is free to execute other tasks.  The iRFE driver also provides wrappers for each of the firmware commands for non-blocking calls such as @ref IfxRfe_executeCalibration_start() and @ref IfxRfe_executeCalibration_finish().

@note It is not allowed to execute any SPI commands to CTRX between the @ref IfxRfe_fwCall_executeDirectly_send() and @ref IfxRfe_fwCall_executeDirectly_read() call, otherwise the CRI would not match and the IfxRfe_fwCall_executeDirectly_read function waits for the RFT pin blockingly.

Option 1: Firmware command call by the user without using iRFE wrapper 
```c
    // this involves using the firmware command calls and providing the arguments in the correct format required by the firmware command

    IfxRfe_ctrxRequest_t cmd;
    IfxRfe_asyncHandle_t handle;

    // configuring the payload parameters for the firmware command
    cmd.buffer[1] = (calib_sub_func_id & 0x7FFF);
    cmd.buffer[1] |= ((detail_result & 0x1) << 15);
    cmd.buffer[2] = tx_ch_pow_idx;
    cmd.buffer[3] = ref_temp_idx;
    cmd.buffer[3] |= (limit_temp << 16);
    cmd.payloadLen = 3;

    // Non-blocking send command
    IfxRfe_fwCall_executeDirectly_send(IFXRFE_FW_COMMAND_EXECUTE_CALIBRATION, &cmd, handle);
    
    // Do other tasks

    // Read command. waits for RFT blockingly.
    IfxRfe_fwCall_executeDirectly_read(handle, &spi_res);
```

Option 2: FW command call using iRFE wrapper 
```c
    // Commands using iRFE wrapper function

    //The handle identifies the command, which is executed.
    IfxRfe_asyncHandle_t handle;

    // call the start command wrapper first
    IfxRfe_executeCalibration_start(*config, &handle);

    // Do other tasks on the controller

    // call the finish command wrapper
    IfxRfe_executeCalibration_finish(handle, result);
```

@subsection FW_cmds_using_handles Configuring and executing firmware command Handles
In this method the user can configure command handles for firmware functions and execute them to reduce the configuration time. The firmware command calls @ref IfxRfe_fwCall_configureHandle, @ref IfxRfe_fwCall_executeHandle and @ref IfxRfe_fwCall_configureAndExecuteHandle configure a handle, execute a handle and configure+execute handle respectively. The iRFE driver provides wrapper function for each of the firmware commands for these commands calls such as @ref IfxRfe_wait_config, @ref IfxRfe_wait_exec and @ref IfxRfe_wait_execConfig. The user can choose to choose to call the firmware command calls themselves or use the wrappers provided by iRFE.

Option 1: Firmware command call by the user without using iRFE wrapper 
```c
    // this involves using the firmware command calls and providing the arguments in the correct format required by the firmware command

    IfxRfe_ctrxRequest_t cmd;
    uint8_t handle;

    // configuring the payload parameters for the firmware command
    cmd.buffer[1] = (calib_sub_func_id & 0x7FFF);
    cmd.buffer[1] |= ((detail_result & 0x1) << 15);
    cmd.buffer[2] = tx_ch_pow_idx;
    cmd.buffer[3] = ref_temp_idx;
    cmd.buffer[3] |= (limit_temp << 16);
    cmd.payloadLen = 3;

    IfxRfe_Config_Update = 0;
    IfxRfe_Config_New    = 1;

    // configuring the handle for Execute_Calibration() FW command using the FW command call
    IfxRfe_fwCall_configureHandle(IFXRFE_FW_COMMAND_EXECUTE_CALIBRATION, IfxRfe_Config_New, &cmd, handle);

    // Other task or SPI commands can be executed

    // Executing the handle configured above for Execute_Calibration() FW command using the FW command call
    IfxRfe_ctrxResponse_t rsp;
    uint8_t receivedCmd;

    IfxRfe_fwCall_executeHandle(handle, &receivedCmd, &rsp);

    // Other task or SPI commands can be executed

    // The command can be executed again using the same handle.

    // updating the payload parameter to change the configuration of an already configured command
    cmd.buffer[2] = tx_ch_pow_idx+1;

    // Updating the configured handle for Execute_Calibration() FW command using the FW command call 
    IfxRfe_fwCall_configureHandle(handle, IfxRfe_Config_Update, &cmd, NULL);
```

Option 2: FW command call using iRFE wrapper 
```c
    // Configuring Handle using iRFE wrapper function

    //The handle identifies the configured command.
    uint8_t handle;

    // Configure the command for starting later. Gives us a handle back to reference it.
    IfxRfe_executeCalibration_config(*config, &handle);    

    // Other task or SPI commands can be executed

    // Now execute the command that is already configured
    IfxRfe_executeCalibration_exec(handle, result);

    // Other task or SPI commands can be executed

    // The command can be executed again using the same handle.

    // To change the configuration of an already configured command, the following function is used:
    IfxRfe_executeCalibration_configUpdate(handle, *config);
```





