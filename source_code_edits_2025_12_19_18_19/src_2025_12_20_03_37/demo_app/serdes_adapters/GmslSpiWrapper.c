#include <errno.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "GmslSpiWrapper.h"
#include "IfxRfe_ErrorDefinitions.h"
#include "PlatformGpio.h"
#include "PlatformLogCallbacks.h"
#include "PlatformSpi.h"

#include "Util.h"

#define MAX_DEVICES       4   ///< Maximum number of supported SPI devices
#define BURST_LEN         12  ///< Burst length for SPI transfers. Must be a multiple of 4 and max. 15.
#define SPI_BITS_PER_WORD 32
#define SPI_SPEED_HZ      5000000
#define BNE_WAIT_TIME     1000

static uint16_t const SPI_CS_NO_GPIO = 0xFFFF;

static uint16_t gRoGmslGpioId       = SPI_CS_NO_GPIO;  ///< GPIO ID for the Read Only (RO) pin
static uint16_t gBneGmslGpioId      = SPI_CS_NO_GPIO;  ///< GPIO ID for the Buffer Not Empty (BNE) pin
static uint8_t gDevIds[MAX_DEVICES] = {0};             ///< Array of device IDs for the SPI devices
static uint8_t gSpiIds[MAX_DEVICES] = {0};             ///< Array of SPI IDs for the SPI devices
static uint8_t gDevCnt              = 0;               ///< Number of SPI devices

int GmslSpiWrapper_setConfig(uint8_t *devIds, uint8_t devCnt, uint16_t ro, uint16_t bne)
{
    if (devIds == NULL || devCnt == 0)
    {
        return IFXRFE_E_FAILED;
    }

    gRoGmslGpioId  = ro;
    gBneGmslGpioId = bne;
    gDevCnt        = devCnt;

    if (gDevCnt > MAX_DEVICES)
    {
        gDevCnt = MAX_DEVICES;  // Limit the number of devices to the maximum supported
    }

    for (uint8_t i = 0; i < gDevCnt; i++)
    {
        // Copy device IDs
        gDevIds[i] = devIds[i];
        // Same for the SPI IDs
        gSpiIds[i] = 0x10;  // Always use SPI device 1.0
    }

    PlatformSpi_init();

    // Set the device IDs
    PlatformSpi_setDevIdSpiIdMapping(gDevIds, gSpiIds, gDevCnt);

    return IFXRFE_E_SUCCESS;
}

int GmslSpiWrapper_configure(uint8_t spiId, uint8_t flags, uint32_t speed)
{
    return PlatformSpi_configure(spiId, flags, speed);
}

int GmslSpiWrapper_write(uint8_t spiId, uint32_t count, const uint32_t buffer[], bool keepSel)
{
    DBG_PRINTF_L1("DEBUG from %s() at %s:%d:\n", __func__, __FILE__, __LINE__);
    return GmslSpiWrapper_transfer(spiId, count, buffer, NULL, keepSel);
}

int GmslSpiWrapper_transfer(uint8_t spiId, uint32_t count, const uint32_t bufWrite[], uint32_t bufRead[],
                            bool keepSel)
{
    uint32_t len                        = count * sizeof(uint32_t);  // In bytes
    uint32_t temp_send_buf[16 / 4]      = {0};                       // /4 because we use uint32_t
    uint32_t temp_recv_buf[32 / 4]      = {0};
    uint32_t temp_read_send_buf[32 / 4] = {0xFF};
    uint32_t temp_read_recv_buf[32 / 4] = {0};
    bool temp_state;
    int tmp_len;
    const uint32_t link_cmd = 0xA0;
    const uint32_t ss_cmd   = (0 == spiId) ? 0xA4 : 0xA5;

    // Clear the rx buffer
    temp_read_send_buf[0] = 0;  // Should not contain 0xA6 to avoid deasserting SS (for keep_sel)
    RETURN_ON_IFX_ERROR(PlatformGpio_set(gRoGmslGpioId, true));
    RETURN_ON_IFX_ERROR(PlatformGpio_get(gBneGmslGpioId, &temp_state));
    if (0 != temp_state)
    {
        RETURN_ON_IFX_ERROR(PlatformSpi_transfer8(spiId, sizeof(temp_read_send_buf), (uint8_t *)temp_read_send_buf, (uint8_t *)temp_read_recv_buf, false));
    }
    // Check BNE again. If still high, error out
    RETURN_ON_IFX_ERROR(PlatformGpio_get(gBneGmslGpioId, &temp_state));
    if (0 != temp_state)
    {
        DBG_PRINTF_L1("SPI BNE should be low, but it's high\n");
        return IFXRFE_E_FAILED;
    }

    // Enter cmd/read mode
    // Set RO
    RETURN_ON_IFX_ERROR(PlatformGpio_set(gRoGmslGpioId, true));
    // Send link_cmd and ss_cmd
    memset(temp_send_buf, 0, sizeof(temp_send_buf));
    memset(temp_recv_buf, 0, sizeof(temp_recv_buf));
    temp_send_buf[0] = (link_cmd << 0) | (ss_cmd << 8);
    RETURN_ON_IFX_ERROR(PlatformSpi_transfer8(spiId, 2, (uint8_t *)temp_send_buf, (uint8_t *)temp_recv_buf, false));

    // Burst mode - BURST_LEN bytes at a time (if needed)
    // Maintain a maximum BIT less than the maximum buffer size (16 bytes) (GMSL2 general UG 18.3.3 SPI Burst Read/Write)
    for (int i = 0; i < len; i += BURST_LEN)
    {
        // Clear RO
        RETURN_ON_IFX_ERROR(PlatformGpio_set(gRoGmslGpioId, false));

        memset(temp_send_buf, 0, sizeof(temp_send_buf));
        memset(temp_recv_buf, 0, sizeof(temp_recv_buf));

        tmp_len = ((len - i) > BURST_LEN) ? BURST_LEN : (len - i);

        memcpy(temp_send_buf, ((uint8_t *)bufWrite) + i, tmp_len);
        RETURN_ON_IFX_ERROR(PlatformSpi_transfer(spiId, tmp_len / 4, temp_send_buf, temp_recv_buf, keepSel));
        RETURN_ON_IFX_ERROR(PlatformGpio_gpioWait(gBneGmslGpioId, true, BNE_WAIT_TIME));

        // Enter read mode
        RETURN_ON_IFX_ERROR(PlatformGpio_set(gRoGmslGpioId, true));

        memset(temp_read_recv_buf, 0, sizeof(temp_read_recv_buf));
        memset(temp_read_send_buf, 0, sizeof(temp_read_recv_buf));
        RETURN_ON_IFX_ERROR(PlatformSpi_transfer(spiId, tmp_len / 4, temp_read_send_buf, temp_read_recv_buf, keepSel));
        if (bufRead)
        {
            memcpy(((uint8_t *)bufRead) + i, temp_read_recv_buf, tmp_len);
        }
        RETURN_ON_IFX_ERROR(PlatformGpio_gpioWait(gBneGmslGpioId, false, BNE_WAIT_TIME));
    }

    // Clear the rx buffer
    // Important! And deassert SS!
    if (!keepSel)
    {
        temp_read_send_buf[0] = 0xA6;  // Deassert SS
    }
    else
    {
        temp_read_send_buf[0] = 0;  // Do not deassert SS
    }
    RETURN_ON_IFX_ERROR(PlatformGpio_set(gRoGmslGpioId, true));
    RETURN_ON_IFX_ERROR(PlatformGpio_get(gBneGmslGpioId, &temp_state));
    if (0 != temp_state)
    {
        RETURN_ON_IFX_ERROR(PlatformSpi_transfer8(spiId, sizeof(temp_read_send_buf), (uint8_t *)temp_read_send_buf, (uint8_t *)temp_read_recv_buf, false));
    }
    else if (!keepSel)
    {
        // Deassert SS
        RETURN_ON_IFX_ERROR(PlatformSpi_transfer8(spiId, 1, (uint8_t *)temp_read_send_buf, (uint8_t *)temp_read_recv_buf, false));
    }
    // Check BNE again
    RETURN_ON_IFX_ERROR(PlatformGpio_get(gBneGmslGpioId, &temp_state));
    if (0 != temp_state)
    {
        DBG_PRINTF_L1("SPI BNE should be low, but it's high\n");
        return IFXRFE_E_FAILED;
    }

    return IFXRFE_E_SUCCESS;
}

void GmslSpiWrapper_deinit()
{
    DBG_PRINTF_L1("DEBUG from PlatformSpi_deinit!\n");
    uint32_t temp_read_send_buf[32 / 4] = {0xFF};
    uint32_t temp_read_recv_buf[32 / 4] = {0};
    bool temp_state;

    // Clear the rx buffer
    temp_read_send_buf[0] = 0xA6;
    (PlatformGpio_set(gRoGmslGpioId, true));
    // Release SS to get CTRX_RFT to high again
    PlatformSpi_transfer8(0, 1, (uint8_t *)temp_read_send_buf, (uint8_t *)temp_read_recv_buf, false);
    (PlatformGpio_get(gBneGmslGpioId, &temp_state));
    if (0 != temp_state)
    {
        PlatformSpi_transfer8(0, sizeof(temp_read_send_buf), (uint8_t *)temp_read_send_buf, (uint8_t *)temp_read_recv_buf, false);
        (PlatformGpio_get(gBneGmslGpioId, &temp_state));
        if (0 != temp_state)
        {
            DBG_PRINTF_L1("SPI BNE should be low, but it's high\n");
        }
    }

    PlatformSpi_deinit();
}