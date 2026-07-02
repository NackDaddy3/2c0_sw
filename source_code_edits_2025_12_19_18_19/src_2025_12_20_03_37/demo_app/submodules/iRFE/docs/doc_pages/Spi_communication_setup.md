@page Spi_communication_setup Spi communication setup
 
The Spi communication between CTRX and Aurix can be setup using the Aurix iLLD drivers and iRFE.
The iLLD drivers for Aurix provide an interface to configure the QSPI which is described in detail @ref IfxLld_Qspi.

@section SPI_using_iLLD SPI communication using iLLD

To setup the SPI communication between the Aurix and CTRX, the iLLD driver for QSPI is utilised.
During the setup Aurix needs to defined as the SPI master and CTRX would work as the SPI slave.

The steps to configure aurix as an SPI master are described in detail at @ref IfxLld_Qspi_SpiMaster_Usage


@section Example_SPI Example using PlatformSpi

The @ref PlatformSpi.c provides a detailed example of a setup of Aurix as an SPI master and setup of the SPI communication between Aurix and CTRX using the iLLD driver and iRFE driver.
The example code included with the driver also uses this implementation of PlatfromSpi to setup the SPI communication.

We can follow the steps mentioned in @ref IfxLld_Qspi_SpiMaster_Usage and see how PlatformSpi implements the same for its SPI communication. 

@subsection Preparation

### Include files
Includes the QSPI master header file:
```c
#include <Qspi/SpiMaster/IfxQspi_SpiMaster.h>
```
Includes the Configuration of Interrupt service routine header file: 
```c
#include <ConfigurationIsr.h>
```
which defines the priorities for QSPI interrupt handlers:
```c 
/* QSPI interrupt priorities */
#define IFX_INTPRIO_QSPI0_TX  0x60 // DMA channel 1
#define IFX_INTPRIO_QSPI0_RX  0x61 // DMA channel 2
#define IFX_INTPRIO_QSPI0_ER  0x62

#define IFX_INTPRIO_QSPI1_TX  0x63 // DMA channel 3
#define IFX_INTPRIO_QSPI1_RX  0x64 // DMA channel 4
#define IFX_INTPRIO_QSPI1_ER  0x65
```


### Variables
Declare QSPI module and Channel handles
```c
// Master handlers (one handler can be used by one or more Spi-devices)
static IfxQspi_SpiMaster m_masterHandlers[PLATFORM_SPI_MAX_MASTERS];
// Device handlers (one handler needed per Spi-device)
static IfxQspi_SpiMaster_Channel m_deviceHandlers[PLATFORM_SPI_MAX_DEVICES];

//Configuration of constructed spi-masters
static const PlatformSpiDefinition_Master *m_masterDefinition[PLATFORM_SPI_MAX_MASTERS] = {NULL};

static IfxDma_ChannelId m_currentDmaChannel = IfxDma_ChannelId_1;
```


### Interrupt Handler installation(with DMA use)
Adds Interrupt Service routines.
```c
static void PlatformSpi_txIsr(uint8_t masterId)
{
    IfxQspi_SpiMaster_isrDmaTransmit(&m_masterHandlers[masterId]);
}

static void PlatformSpi_rxIsr(uint8_t masterId)
{
    IfxQspi_SpiMaster_isrDmaReceive(&m_masterHandlers[masterId]);
    if (m_callbackFn[masterId] != NULL)
    {
        m_callbackFn[masterId](m_callbackArg[masterId]);
        m_callbackFn[masterId] = NULL;
    }
}

#if IFX_INTPRIO_QSPI0_ER
IFX_INTERRUPT(ISR_qspi0_Er, 0, IFX_INTPRIO_QSPI0_ER);

void ISR_qspi0_Er(void)
{
    IfxQspi_SpiMaster_isrError(&m_masterHandlers[0]);
}
#endif

#if IFX_INTPRIO_QSPI0_TX
IFX_INTERRUPT(ISR_qspi0_Tx, 0, IFX_INTPRIO_QSPI0_TX);

void ISR_qspi0_Tx(void)
{
    PlatformSpi_txIsr(0);
}
#endif

#if IFX_INTPRIO_QSPI0_RX
IFX_INTERRUPT(ISR_qspi0_Rx, 0, IFX_INTPRIO_QSPI0_RX);

void ISR_qspi0_Rx(void)
{
    PlatformSpi_rxIsr(0);
}
#endif
```


### module initialization (with dma)
Module initialization for Master mode is available using the method PlatformSpi_initMasterHandler
```c
static void PlatformSpi_initMasterHandler(const PlatformSpiDefinition_Master *masterConfig, IfxQspi_SpiMaster *masterHandler)
{
    masterHandler->qspi = masterConfig->mclk->module;

    // create module default-config
    IfxQspi_SpiMaster_Config masterHandlerConfig;
    IfxQspi_SpiMaster_initModuleConfig(&masterHandlerConfig, masterHandler->qspi);

    // set the desired mode and maximum speed
    masterHandlerConfig.base.mode            = SpiIf_Mode_master;
    masterHandlerConfig.base.maximumBaudrate = masterConfig->maxSpeed;

    // ISR priorities and interrupt target
    const IfxQspi_Index masterId           = IfxQspi_getIndex(masterConfig->mclk->module);
    masterHandlerConfig.base.txPriority    = AURIXSPI_IRQ_PRIORITY_TX[masterId];
    masterHandlerConfig.base.rxPriority    = AURIXSPI_IRQ_PRIORITY_RX[masterId];
    masterHandlerConfig.base.erPriority    = AURIXSPI_IRQ_PRIORITY_ER[masterId];
    masterHandlerConfig.base.isrProvider   = IfxCpu_Irq_getTos(IfxCpu_getCoreIndex());
    masterHandlerConfig.dma.txDmaChannelId = m_currentDmaChannel++;
    masterHandlerConfig.dma.rxDmaChannelId = m_currentDmaChannel++;
    masterHandlerConfig.dma.useDma         = TRUE;

    // pin configuration
    const IfxQspi_SpiMaster_Pins pins = {
        masterConfig->mclk,
        IfxPort_OutputMode_pushPull,
        masterConfig->mosi,
        IfxPort_OutputMode_pushPull,
        masterConfig->miso,
        IfxPort_InputMode_pullDown,
        IfxPort_PadDriver_cmosAutomotiveSpeed1,
    };
    masterHandlerConfig.pins = &pins;

    // initialize module
    IfxQspi_SpiMaster_initModule(masterHandler, &masterHandlerConfig);

    if (AURIXSPI_IRQ_PRIORITY_ER[masterId])
    {
        volatile Ifx_SRC_SRCR *src = IfxQspi_getErrorSrc(masterHandlerConfig.qspi);
        IfxSrc_init(src, (IfxSrc_Tos)IfxCpu_getCoreIndex(), AURIXSPI_IRQ_PRIORITY_ER[masterId]);
        IfxSrc_enable(src);
    }
}
```
### SPI channel Initialization
SPI channel configuration in master mode:
```c
static void PlatformSpi_initDeviceHandler(const PlatformSpiDefinition_Device *deviceConfig,
                                          IfxQspi_SpiMaster_Channel *deviceHandler, const IfxQspi_SpiMaster *masterHandler,
                                          uint8_t flags, uint8_t wordSize, float speed)
{
    // create default deviceHandler config
    IfxQspi_SpiMaster_ChannelConfig deviceHandlerConfig;
    IfxQspi_SpiMaster_initChannelConfig(&deviceHandlerConfig, (IfxQspi_SpiMaster *)masterHandler);

    deviceHandlerConfig.dummyTxValue = 0;

    // specific settings for this device
    deviceHandlerConfig.base.baudrate           = speed;
    deviceHandlerConfig.base.mode.autoCS        = 0;
    deviceHandlerConfig.base.mode.dataWidth     = wordSize;
    deviceHandlerConfig.base.mode.clockPolarity = (flags & SPI_CPOL) ? SpiIf_ClockPolarity_idleHigh : SpiIf_ClockPolarity_idleLow;
    deviceHandlerConfig.base.mode.shiftClock    = (flags & SPI_CPHA) ? SpiIf_ShiftClock_shiftTransmitDataOnLeadingEdge : SpiIf_ShiftClock_shiftTransmitDataOnTrailingEdge;
    deviceHandlerConfig.base.mode.csActiveLevel = (flags & SPI_CS_HIGH_ACTIVE) ? Ifx_ActiveState_high : Ifx_ActiveState_low;
    deviceHandlerConfig.base.mode.dataHeading   = (flags & SPI_LSB_FIRST) ? SpiIf_DataHeading_lsbFirst : SpiIf_DataHeading_msbFirst;

    // select pin configuration
    deviceHandlerConfig.sls.output.pin    = deviceConfig->ssel;
    deviceHandlerConfig.sls.output.mode   = IfxPort_OutputMode_pushPull;
    deviceHandlerConfig.sls.output.driver = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    // initialize deviceHandler
    IfxQspi_SpiMaster_initChannel(deviceHandler, &deviceHandlerConfig);
}
```

### Data Transfers
A method PlatformSpi_dataTransferBlocking is provided for Data transfer using QSPI.
```c
static sr_t PlatformSpi_dataTransferBlocking(uint8_t devId, uint32_t count, const void *bufWrite, void *bufRead, bool keepSel)
{
    if (devId >= m_deviceCount)
    {
        return E_OUT_OF_BOUNDS;
    }
    if (!m_configured[devId])
    {
        return E_NOT_CONFIGURED;
    }

    IfxQspi_SpiMaster_Channel *deviceHandler = &m_deviceHandlers[devId];
    Ifx_TickTime deadline                    = getDeadLine(SPI_TRANSFER_TIMEOUT);

    //Wait until it is available
    while (IfxQspi_SpiMaster_getStatus(deviceHandler) != SpiIf_Status_ok)
    {
        if (isDeadLine(deadline))
        {
            return E_TIMEOUT;
        }
    }

    // set configured data width in case it was changed
    deviceHandler->dataWidth  = m_wordSize[devId];
    deviceHandler->bacon.B.DL = m_wordSize[devId] - 1;

    IfxQspi_SpiMaster_AutoSlso backup;
    if (keepSel)
    {
        backup                        = deviceHandler->deactivateSlso;
        deviceHandler->deactivateSlso = NULL_PTR;
    }
    IfxQspi_SpiMaster_exchange(deviceHandler, bufWrite, bufRead, count);

    //Wait until it is done
    sr_t ret = E_SUCCESS;
    deadline = getDeadLine(SPI_TRANSFER_TIMEOUT);
    while (IfxQspi_SpiMaster_getStatus(deviceHandler) != SpiIf_Status_ok)
    {
        if (isDeadLine(deadline))
        {
            ret = E_TIMEOUT;
            break;
        }
    }

    if (keepSel)
    {
        deviceHandler->deactivateSlso = backup;
        if (ret)
        {
            deviceHandler->deactivateSlso(deviceHandler);
        }
    }

    return ret;
}
```