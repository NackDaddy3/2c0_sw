#include "../Platform/PlatformSpi.h"
#include <ConfigurationIsr.h>

#include <stdbool.h>
#include <stddef.h>

#include <IfxDma_cfg.h>
#include <Qspi/SpiMaster/IfxQspi_SpiMaster.h>
#include "AurixTime.h"
#include "ConfigurationIsr.h"

#define USE_SPI_DMA

/******************************************************************************/
/*Macro Definitions ----------------------------------------------------------*/
/******************************************************************************/
#ifndef PLATFORM_SPI_MAX_DEVICES
#define PLATFORM_SPI_MAX_DEVICES 8u
#endif
#define PLATFORM_SPI_MAX_MASTERS IFXQSPI_NUM_MODULES

#define SPI_MAX_TRANSFER_LENGTH 32768u
#define SPI_TRANSFER_TIMEOUT    TimeConst_100ms

/******************************************************************************/
/*Private/Public Constants ---------------------------------------------------*/
/******************************************************************************/
#ifndef IFX_INTPRIO_QSPI0_TX
#define IFX_INTPRIO_QSPI0_TX 0
#endif
#ifndef IFX_INTPRIO_QSPI1_TX
#define IFX_INTPRIO_QSPI1_TX 0
#endif
#ifndef IFX_INTPRIO_QSPI2_TX
#define IFX_INTPRIO_QSPI2_TX 0
#endif
#ifndef IFX_INTPRIO_QSPI3_TX
#define IFX_INTPRIO_QSPI3_TX 0
#endif
#if IFXQSPI_NUM_MODULES > 4
#ifndef IFX_INTPRIO_QSPI4_TX
#define IFX_INTPRIO_QSPI4_TX 0
#endif
#ifndef IFX_INTPRIO_QSPI5_TX
#define IFX_INTPRIO_QSPI5_TX 0
#endif
#endif

#ifndef IFX_INTPRIO_QSPI0_RX
#define IFX_INTPRIO_QSPI0_RX 0
#endif
#ifndef IFX_INTPRIO_QSPI1_RX
#define IFX_INTPRIO_QSPI1_RX 0
#endif
#ifndef IFX_INTPRIO_QSPI2_RX
#define IFX_INTPRIO_QSPI2_RX 0
#endif
#ifndef IFX_INTPRIO_QSPI3_RX
#define IFX_INTPRIO_QSPI3_RX 0
#endif
#if IFXQSPI_NUM_MODULES > 4
#ifndef IFX_INTPRIO_QSPI4_RX
#define IFX_INTPRIO_QSPI4_RX 0
#endif
#ifndef IFX_INTPRIO_QSPI5_RX
#define IFX_INTPRIO_QSPI5_RX 0
#endif
#endif

#ifndef IFX_INTPRIO_QSPI0_ER
#define IFX_INTPRIO_QSPI0_ER 0
#endif
#ifndef IFX_INTPRIO_QSPI1_ER
#define IFX_INTPRIO_QSPI1_ER 0
#endif
#ifndef IFX_INTPRIO_QSPI2_ER
#define IFX_INTPRIO_QSPI2_ER 0
#endif
#ifndef IFX_INTPRIO_QSPI3_ER
#define IFX_INTPRIO_QSPI3_ER 0
#endif
#if IFXQSPI_NUM_MODULES > 4
#ifndef IFX_INTPRIO_QSPI4_ER
#define IFX_INTPRIO_QSPI4_ER 0
#endif
#ifndef IFX_INTPRIO_QSPI5_ER
#define IFX_INTPRIO_QSPI5_ER 0
#endif
#endif

static const Ifx_Priority AURIXSPI_IRQ_PRIORITY_TX[IFXQSPI_NUM_MODULES] = {
    IFX_INTPRIO_QSPI0_TX,
    IFX_INTPRIO_QSPI1_TX,
    IFX_INTPRIO_QSPI2_TX,
    IFX_INTPRIO_QSPI3_TX,
#if IFXQSPI_NUM_MODULES > 4
    IFX_INTPRIO_QSPI4_TX,
    IFX_INTPRIO_QSPI5_TX,
#endif
};

static const Ifx_Priority AURIXSPI_IRQ_PRIORITY_RX[IFXQSPI_NUM_MODULES] = {
    IFX_INTPRIO_QSPI0_RX,
    IFX_INTPRIO_QSPI1_RX,
    IFX_INTPRIO_QSPI2_RX,
    IFX_INTPRIO_QSPI3_RX,
#if IFXQSPI_NUM_MODULES > 4
    IFX_INTPRIO_QSPI4_RX,
    IFX_INTPRIO_QSPI5_RX,
#endif
};

static const Ifx_Priority AURIXSPI_IRQ_PRIORITY_ER[IFXQSPI_NUM_MODULES] = {
    IFX_INTPRIO_QSPI0_ER,
    IFX_INTPRIO_QSPI1_ER,
    IFX_INTPRIO_QSPI2_ER,
    IFX_INTPRIO_QSPI3_ER,
#if IFXQSPI_NUM_MODULES > 4
    IFX_INTPRIO_QSPI4_ER,
    IFX_INTPRIO_QSPI5_ER,
#endif
};

/******************************************************************************/
/*Private/Public Variables ---------------------------------------------------*/
/******************************************************************************/
//Configuration of constructed spi-devices
static const PlatformSpiDefinition_Device *m_deviceDefinition;
static unsigned int m_deviceCount                  = 0;
static bool m_configured[PLATFORM_SPI_MAX_DEVICES] = {false};

// Master handlers (one handler can be used by one or more Spi-devices)
static IfxQspi_SpiMaster m_masterHandlers[PLATFORM_SPI_MAX_MASTERS];
// Device handlers (one handler needed per Spi-device)
static IfxQspi_SpiMaster_Channel m_deviceHandlers[PLATFORM_SPI_MAX_DEVICES];
static uint32_t m_deviceTimeouts[PLATFORM_SPI_MAX_DEVICES];

//Configuration of constructed spi-masters
static const PlatformSpiDefinition_Master *m_masterDefinition[PLATFORM_SPI_MAX_MASTERS] = {NULL};

#ifdef USE_SPI_DMA
static IfxDma_ChannelId m_currentDmaChannel = IfxDma_ChannelId_1;

static void (*m_callbackFn[PLATFORM_SPI_MAX_MASTERS])(void *) = {NULL}; /** \brief function to be called when transfer is complete */
static void *m_callbackArg[PLATFORM_SPI_MAX_MASTERS];                   /** \brief context for callback */
#endif
static uint8_t m_wordSize[PLATFORM_SPI_MAX_DEVICES];


/******************************************************************************/
/*Private Methods Declaration ------------------------------------------------*/
/******************************************************************************/


/******************************************************************************/
/*Private Methods Definition -------------------------------------------------*/
/******************************************************************************/

static void PlatformSpi_txIsr(uint8_t masterId)
{
#ifdef USE_SPI_DMA
    IfxQspi_SpiMaster_isrDmaTransmit(&m_masterHandlers[masterId]);
#else
    IfxQspi_SpiMaster_isrTransmit(&m_masterHandlers[masterId]);
#endif
}

static void PlatformSpi_rxIsr(uint8_t masterId)
{
#ifdef USE_SPI_DMA
    IfxQspi_SpiMaster_isrDmaReceive(&m_masterHandlers[masterId]);
    if (m_callbackFn[masterId] != NULL)
    {
        m_callbackFn[masterId](m_callbackArg[masterId]);
        m_callbackFn[masterId] = NULL;
    }
#else
    IfxQspi_SpiMaster_isrReceive(&m_masterHandlers[masterId]);
#endif
}

#if IFX_INTPRIO_QSPI0_ER
IFX_INTERRUPT(ISR_qspi0_Er, 0, IFX_INTPRIO_QSPI0_ER);

void ISR_qspi0_Er(void)
{
    IfxQspi_SpiMaster_isrError(&m_masterHandlers[0]);
}
#endif


#if IFX_INTPRIO_QSPI1_ER
IFX_INTERRUPT(ISR_qspi1_Er, 0, IFX_INTPRIO_QSPI1_ER);

void ISR_qspi1_Er(void)
{
    IfxQspi_SpiMaster_isrError(&m_masterHandlers[1]);
}
#endif


#if IFX_INTPRIO_QSPI2_ER
IFX_INTERRUPT(ISR_qspi2_Er, 0, IFX_INTPRIO_QSPI2_ER);

void ISR_qspi2_Er(void)
{
    IfxQspi_SpiMaster_isrError(&m_masterHandlers[2]);
}
#endif


#if IFX_INTPRIO_QSPI3_ER
IFX_INTERRUPT(ISR_qspi3_Er, 0, IFX_INTPRIO_QSPI3_ER);

void ISR_qspi3_Er(void)
{
    IfxQspi_SpiMaster_isrError(&m_masterHandlers[3]);
}
#endif


#if IFXQSPI_NUM_MODULES > 4

#if IFX_INTPRIO_QSPI4_ER
IFX_INTERRUPT(ISR_qspi4_Er, 0, IFX_INTPRIO_QSPI4_ER);

void ISR_qspi4_Er(void)
{
    IfxQspi_SpiMaster_isrError(&m_masterHandlers[4]);
}
#endif


#if IFX_INTPRIO_QSPI5_ER
IFX_INTERRUPT(ISR_qspi5_Er, 0, IFX_INTPRIO_QSPI5_ER);

void ISR_qspi5_Er(void)
{
    IfxQspi_SpiMaster_isrError(&m_masterHandlers[5]);
}
#endif
#endif


#if IFX_INTPRIO_QSPI0_TX
IFX_INTERRUPT(ISR_qspi0_Tx, 0, IFX_INTPRIO_QSPI0_TX);

void ISR_qspi0_Tx(void)
{
    PlatformSpi_txIsr(0);
}
#endif

#if IFX_INTPRIO_QSPI1_TX
IFX_INTERRUPT(ISR_qspi1_Tx, 0, IFX_INTPRIO_QSPI1_TX);

void ISR_qspi1_Tx(void)
{
    PlatformSpi_txIsr(1);
}
#endif

#if IFX_INTPRIO_QSPI2_TX
IFX_INTERRUPT(ISR_qspi2_Tx, 0, IFX_INTPRIO_QSPI2_TX);

void ISR_qspi2_Tx(void)
{
    PlatformSpi_txIsr(2);
}
#endif

#if IFX_INTPRIO_QSPI3_TX
IFX_INTERRUPT(ISR_qspi3_Tx, 0, IFX_INTPRIO_QSPI3_TX);

void ISR_qspi3_Tx(void)
{
    PlatformSpi_txIsr(3);
}
#endif

#if IFXQSPI_NUM_MODULES > 4
#if IFX_INTPRIO_QSPI4_TX
IFX_INTERRUPT(ISR_qspi4_Tx, 0, IFX_INTPRIO_QSPI4_TX);

void ISR_qspi4_Tx(void)
{
    PlatformSpi_txIsr(4);
}
#endif

#if IFX_INTPRIO_QSPI5_TX
IFX_INTERRUPT(ISR_qspi5_Tx, 0, IFX_INTPRIO_QSPI5_TX);

void ISR_qspi5_Tx(void)
{
    PlatformSpi_txIsr(5);
}
#endif
#endif


#if IFX_INTPRIO_QSPI0_RX
IFX_INTERRUPT(ISR_qspi0_Rx, 0, IFX_INTPRIO_QSPI0_RX);

void ISR_qspi0_Rx(void)
{
    PlatformSpi_rxIsr(0);
}
#endif

#if IFX_INTPRIO_QSPI1_RX
IFX_INTERRUPT(ISR_qspi1_Rx, 0, IFX_INTPRIO_QSPI1_RX);

void ISR_qspi1_Rx(void)
{
    PlatformSpi_rxIsr(1);
}
#endif

#if IFX_INTPRIO_QSPI2_RX
IFX_INTERRUPT(ISR_qspi2_Rx, 0, IFX_INTPRIO_QSPI2_RX);

void ISR_qspi2_Rx(void)
{
    PlatformSpi_rxIsr(2);
}
#endif

#if IFX_INTPRIO_QSPI3_RX
IFX_INTERRUPT(ISR_qspi3_Rx, 0, IFX_INTPRIO_QSPI3_RX);

void ISR_qspi3_Rx(void)
{
    PlatformSpi_rxIsr(3);
}
#endif

#if IFXQSPI_NUM_MODULES > 4
#if IFX_INTPRIO_QSPI4_RX
IFX_INTERRUPT(ISR_qspi4_Rx, 0, IFX_INTPRIO_QSPI4_RX);

void ISR_qspi4_Rx(void)
{
    PlatformSpi_rxIsr(4);
}
#endif

#if IFX_INTPRIO_QSPI5_RX
IFX_INTERRUPT(ISR_qspi5_Rx, 0, IFX_INTPRIO_QSPI5_RX);

void ISR_qspi5_Rx(void)
{
    PlatformSpi_rxIsr(5);
}
#endif
#endif


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
    const IfxQspi_Index masterId         = IfxQspi_getIndex(masterConfig->mclk->module);
    masterHandlerConfig.base.txPriority  = AURIXSPI_IRQ_PRIORITY_TX[masterId];
    masterHandlerConfig.base.rxPriority  = AURIXSPI_IRQ_PRIORITY_RX[masterId];
    masterHandlerConfig.base.erPriority  = AURIXSPI_IRQ_PRIORITY_ER[masterId];
    masterHandlerConfig.base.isrProvider = IfxCpu_Irq_getTos(IfxCpu_getCoreIndex());
#ifdef USE_SPI_DMA
    masterHandlerConfig.dma.txDmaChannelId = m_currentDmaChannel++;
    masterHandlerConfig.dma.rxDmaChannelId = m_currentDmaChannel++;
    masterHandlerConfig.dma.useDma         = TRUE;
#endif

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
    Ifx_TickTime deadline                    = getDeadLine(m_deviceTimeouts[devId]);

    //Wait until it is available
    while (IfxQspi_SpiMaster_getStatus(deviceHandler) != SpiIf_Status_ok)
    {
        if (isDeadLine(deadline))
        {
            return E_TIMEOUT;
        }
    }

#ifdef USE_SPI_DMA
    // set configured data width in case it was changed
    deviceHandler->dataWidth  = m_wordSize[devId];
    deviceHandler->bacon.B.DL = m_wordSize[devId] - 1;
#endif

    IfxQspi_SpiMaster_AutoSlso backup;
    if (keepSel)
    {
        backup                        = deviceHandler->deactivateSlso;
        deviceHandler->deactivateSlso = NULL_PTR;
    }
    IfxQspi_SpiMaster_exchange(deviceHandler, bufWrite, bufRead, count);

    //Wait until it is done
    sr_t ret = E_SUCCESS;
    deadline = getDeadLine(m_deviceTimeouts[devId]);
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

#ifdef USE_SPI_DMA
static sr_t PlatformSpi_dataTransferAsync(uint8_t devId, uint32_t count, const void *bufWrite, void *bufRead, uint8_t wordSize)
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
    Ifx_TickTime deadline                    = getDeadLine(m_deviceTimeouts[devId]);

    //Wait until it is available
    while (IfxQspi_SpiMaster_getStatus(deviceHandler) != SpiIf_Status_ok)
    {
        if (isDeadLine(deadline))
        {
            return E_TIMEOUT;
        }
    }

    // set data width in case burst transfer word size differs from configured
    deviceHandler->dataWidth  = wordSize;
    deviceHandler->bacon.B.DL = wordSize - 1;

    IfxQspi_SpiMaster_exchange(deviceHandler, bufWrite, bufRead, count);

    return E_SUCCESS;
}
#endif

static sr_t PlatformSpi_readBurstAsync(uint8_t devId, void *bufRead, uint32_t count, uint8_t wordSize, void (*cb)(void *), void *arg)
{
#ifdef USE_SPI_DMA
    if (devId >= m_deviceCount)
    {
        return E_OUT_OF_BOUNDS;
    }
    if (!m_configured[devId])
    {
        return E_NOT_CONFIGURED;
    }

    const PlatformSpiDefinition_Device *deviceDefinition = &m_deviceDefinition[devId];
    const IfxQspi_Index masterId                         = IfxQspi_getIndex(deviceDefinition->ssel->module);

    m_callbackFn[masterId]  = cb;
    m_callbackArg[masterId] = arg;

    sr_t ret = PlatformSpi_dataTransferAsync(devId, count, NULL, bufRead, wordSize);

    if (ret != E_SUCCESS)
    {
        m_callbackFn[masterId] = NULL;
    }
    return ret;
#else
    return E_NOT_IMPLEMENTED;
#endif
}

/******************************************************************************/
/*Interface Methods Definition -----------------------------------------------*/
/******************************************************************************/
sr_t PlatformSpi_readBurstAsync8(uint8_t devId, uint8_t *bufRead, uint32_t count, void (*cb)(void *), void *arg)
{
    return PlatformSpi_readBurstAsync(devId, bufRead, count, 8, cb, arg);
}

sr_t PlatformSpi_readBurstAsync12(uint8_t devId, uint16_t *bufRead, uint32_t count, void (*cb)(void *), void *arg)
{
    return PlatformSpi_readBurstAsync(devId, bufRead, count, 12, cb, arg);
}

uint32_t PlatformSpi_getMaxTransfer(void)
{
    return SPI_MAX_TRANSFER_LENGTH;
}

sr_t PlatformSpi_configure(uint8_t devId, uint8_t flags, uint8_t wordSize, uint32_t speed)
{
    if (devId >= m_deviceCount)
    {
        return E_OUT_OF_BOUNDS;
    }

    const PlatformSpiDefinition_Device *deviceDefinition = &m_deviceDefinition[devId];
    const IfxQspi_Index masterId                         = IfxQspi_getIndex(deviceDefinition->ssel->module);
    const IfxQspi_SpiMaster *masterHandler               = &m_masterHandlers[masterId];
    IfxQspi_SpiMaster_Channel *deviceHandler             = &m_deviceHandlers[devId];

    float devSpeed       = (float)speed;
    const float maxSpeed = masterHandler->maximumBaudrate;
    if (devSpeed > maxSpeed)
    {
        // silently clip the device speed to the maximum allowed
        devSpeed = maxSpeed;
    }

    PlatformSpi_initDeviceHandler(deviceDefinition, deviceHandler, masterHandler, flags, wordSize, devSpeed);
    IfxQspi_SpiMaster_setChannelBaudrate(deviceHandler, devSpeed);

    PlatformSpi_setTimeout(devId, SPI_TRANSFER_TIMEOUT);

    m_wordSize[devId]   = wordSize;
    m_configured[devId] = true;
    return E_SUCCESS;
}

sr_t PlatformSpi_write8(uint8_t devId, uint32_t count, const uint8_t buffer[], bool keepSel)
{
    //Input buffer is NULL -> do not care about incoming data
    return PlatformSpi_dataTransferBlocking(devId, count, buffer, NULL, keepSel);
}

sr_t PlatformSpi_write16(uint8_t devId, uint32_t count, const uint16_t buffer[], bool keepSel)
{
    //Input buffer is NULL -> do not care about incoming data
    return PlatformSpi_transfer16(devId, count, buffer, NULL, keepSel);  // TODO: revert once hack that resembles that Hatvan SPI bug is removed
}

sr_t PlatformSpi_write32(uint8_t devId, uint32_t count, const uint32_t buffer[], bool keepSel)
{
    //Input buffer is NULL -> do not care about incoming data
    return PlatformSpi_dataTransferBlocking(devId, count, buffer, NULL, keepSel);
}

sr_t PlatformSpi_read8(uint8_t devId, uint32_t count, uint8_t buffer[], bool keepSel)
{
    //Output buffer is NULL -> send all ones at every transfer
    return PlatformSpi_dataTransferBlocking(devId, count, NULL, buffer, keepSel);
}

sr_t PlatformSpi_read16(uint8_t devId, uint32_t count, uint16_t buffer[], bool keepSel)
{
    //Output buffer is NULL -> send all ones at every transfer
    return PlatformSpi_transfer16(devId, count, NULL, buffer, keepSel);  // TODO: revert once hack that resembles that Hatvan SPI bug is removed
}

sr_t PlatformSpi_read32(uint8_t devId, uint32_t count, uint32_t buffer[], bool keepSel)
{
    //Output buffer is NULL -> send all ones at every transfer
    return PlatformSpi_dataTransferBlocking(devId, count, NULL, buffer, keepSel);
}

sr_t PlatformSpi_transfer8(uint8_t devId, uint32_t count, const uint8_t bufWrite[], uint8_t bufRead[], bool keepSel)
{
    return PlatformSpi_dataTransferBlocking(devId, count, bufWrite, bufRead, keepSel);
}

sr_t PlatformSpi_transfer16(uint8_t devId, uint32_t count, const uint16_t bufWrite[], uint16_t bufRead[], bool keepSel)
{
    return PlatformSpi_dataTransferBlocking(devId, count, bufWrite, bufRead, keepSel);
}

sr_t PlatformSpi_transfer32(uint8_t devId, uint32_t count, const uint32_t bufWrite[], uint32_t bufRead[], bool keepSel)
{
    return PlatformSpi_dataTransferBlocking(devId, count, bufWrite, bufRead, keepSel);
}

sr_t PlatformSpi_writeQspi8(uint8_t devId, uint32_t count, const uint8_t buffer[], bool keepSel)
{
    return E_NOT_IMPLEMENTED;
}

sr_t PlatformSpi_readQspi8(uint8_t devId, uint32_t count, uint8_t buffer[], bool keepSel)
{
    return E_NOT_IMPLEMENTED;
}

sr_t PlatformSpi_switchDevicePins(uint8_t devId, bool state)
{
    if (devId >= m_deviceCount)
    {
        return E_OUT_OF_BOUNDS;
    }

    const IfxQspi_Slso_Out *ssel = m_deviceDefinition[devId].ssel;
    const IfxQspi_Index masterId = IfxQspi_getIndex(ssel->module);
    if (m_masterDefinition[masterId] == NULL)
    {
        return E_INVALID_PARAMETER;
    }

    const IfxQspi_Sclk_Out *mclk = m_masterDefinition[masterId]->mclk;
    const IfxQspi_Mtsr_Out *mosi = m_masterDefinition[masterId]->mosi;
    const IfxQspi_Mrst_In *miso  = m_masterDefinition[masterId]->miso;

    if (state == true)  //enable pins
    {
        IfxPort_setPinModeOutput(mclk->pin.port, mclk->pin.pinIndex, IfxPort_OutputMode_pushPull, mclk->select);
        IfxPort_setPinModeOutput(mosi->pin.port, mosi->pin.pinIndex, IfxPort_OutputMode_pushPull, mosi->select);
        IfxPort_setPinModeInput(miso->pin.port, miso->pin.pinIndex, IfxPort_InputMode_pullDown);
        IfxPort_setPinModeOutput(ssel->pin.port, ssel->pin.pinIndex, IfxPort_OutputMode_pushPull, ssel->select);
    }
    else  //disable pins
    {
        IfxPort_setPinModeInput(mclk->pin.port, mclk->pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
        IfxPort_setPinModeInput(mosi->pin.port, mosi->pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
        IfxPort_setPinModeInput(miso->pin.port, miso->pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
        IfxPort_setPinModeInput(ssel->pin.port, ssel->pin.pinIndex, IfxPort_Mode_inputNoPullDevice);
    }
    return E_SUCCESS;
}

sr_t PlatformSpi_setTimeout(uint8_t devId, uint32_t timeout)
{
    m_deviceTimeouts[devId] = timeout;
    return E_SUCCESS;
}

sr_t PlatformSpi_initialize(const PlatformSpiDefinition_Master *masters, uint8_t masterCount, const PlatformSpiDefinition_Device *devices, uint8_t deviceCount)
{
    if ((masters == NULL) || (devices == NULL))
    {
        m_deviceCount = 0;
        return E_SUCCESS;
    }
    if ((masterCount > PLATFORM_SPI_MAX_MASTERS) || (deviceCount > PLATFORM_SPI_MAX_DEVICES))
    {
        return E_OUT_OF_BOUNDS;
    }
    m_deviceCount      = deviceCount;
    m_deviceDefinition = devices;

    for (unsigned int i = 0; i < masterCount; i++)
    {
        const IfxQspi_Index masterId = IfxQspi_getIndex(masters[i].mclk->module);
        if (masterId >= IFXQSPI_NUM_MODULES ||
            AURIXSPI_IRQ_PRIORITY_TX[masterId] == 0 ||
            AURIXSPI_IRQ_PRIORITY_RX[masterId] == 0)
        {
            return E_INVALID_PARAMETER;
        }

        //Since i could be different than masterId, it has to be stored as follows:
        m_masterDefinition[masterId] = &masters[i];
        PlatformSpi_initMasterHandler(m_masterDefinition[masterId], &m_masterHandlers[masterId]);
    }

    return E_SUCCESS;
}

/* @} */


