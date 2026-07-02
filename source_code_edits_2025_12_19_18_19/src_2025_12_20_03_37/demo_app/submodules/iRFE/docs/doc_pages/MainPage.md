@mainpage CTRX iRFE (Radar Frontend) driver

## Introduction
The aim of the CTRX iRFE driver is to provide access for the hardware interface, low level SPI commands, firmware commands and convenience functions for Infineon's CTRX family of MMICs developed for automotive radar applications. It is intended for software engineers looking to use the CTRX for developing embedded radar applications. Together with the iLLD Library for Aurix, it allows users to setup and configure the device quickly for their intended applications.


One of the pre-requisites for working with the iRFE Driver is the familiarity of the user with the CTRX user manual. Chapter 2: Interface and Chapter 4: Device configuration in particular must be familiar to the user. The iRFE documentation only covers basic description of the CTRX device, interface and functionality. Detailed descriptions are provided in the User manual.


## Table of Contents
The documentation is divided into pages introducing the user to various aspect of the driver environment.

| Chapter| Title || Description |
|:------- |:-------  | :-------  | :--------- |
| 1.| [About CTRX device](#About_CTRX) || Provides the Hardware details of the device in brief.|
| 2.| [About CTRX iRFE Driver](#About_iRFE_Driver) || Describes the iRFE driver and its architecture. |
| ^ |2.1.| [CTRX Configuration interface](#Configuration_interface)| Describes the configuration interface of the driver. |
| ^ |2.1.1| [Low level commands](#Low_level_commands) | Describes the low level commands available through the driver interface.|
| ^ |2.1.2| [Firmware command calls](#Firmware_command_calls)| Describes the types of firmware command calls available and their usage |
| ^ |2.1.3| [Firmware_commands_8191_A11](#Firmware_commands_8191_A11) | Describes the available firmware commands and associated wrapper functions for 8191_A11|
| ^ |2.1.4| [Firmware_commands_8191_B11](#Firmware_commands_8191_B11) | Describes the available firmware commands and associated wrapper functions for 8191_B11 |
| ^ |2.1.5| [Firmware_commands_8188](#Firmware_commands_8188) | Describes the available firmware commands and associated wrapper functions for 8188|
| ^ |2.2| [Convenience functions](#Convenience_functions)| Describes the available convenience function with iRFE |
| ^ |2.3| [Additional Functions](#Additional_functions) | Describes the additional functions available with iRFE |
| 3.| [Operating CTRX](#Operating_CTRX)|| Describes in brief the operation cycle of CTRX and links to iRFE functions |
| 4.| [Spi communication setup](#Spi_communication_setup) || The setup of SPI communication is briefed in this page.|
| 5.| [Error Handling](#Error_Handling) || Introduces how the errors are handled by the driver. |
| 6.| [Example Code Description](#Example_page) || Describes the Example code packaged with the driver. |


