@page Additional_functions Additional Functions

@section GPIO_Wrapper GPIO Wrapper

Provides functionalities to interface the GPIO pins of the MMIC such as:
- @ref InitGpioWrapper : Sets the Pin Ids
- @ref Wrapper_GpioGet : Get the gpio state
- @ref Wrapper_GpioSet : Set the gpio to a state
- @ref Wrapper_GpioConfigure : Configure gpio
- @ref Wrapper_IsGpioConfigOutput : Determine whether the configuration is output


@section SPI_Wrapper SPI Wrapper

Provide the wrapper to access the platform SPI functionalities:
- @ref InitSpiWrapper
- @ref Wrapper_SpiTransfer
- @ref Wrapper_SpiWrite
- @ref Wrapper_SpiConfigure