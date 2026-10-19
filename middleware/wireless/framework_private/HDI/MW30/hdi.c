/*!
 * Copyright 2025-2026 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * \file hdi.c
 *
 */

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include "hdi.h"
#include "fsl_common.h"
#include "fsl_lpuart.h"
#include "fsl_clock.h"

/* -------------------------------------------------------------------------- */
/*                               Private macros                               */
/* -------------------------------------------------------------------------- */

#ifndef HDI_TEST
#define HDI_TEST 0
#endif

/* Radio board mode selection */
#define HDI_CMD_BLE          0xF3
#define HDI_CMD_ZB           0xF4
#define HDI_CMD_GFSK         0xF5
#define HDI_CMD_CS_INITIATOR 0xF6
#define HDI_CMD_CS_REFLECTOR 0xF7

#if 0
#define HDI_CMD_TERMINATOR 0xFF
#endif

#ifdef HDI_CMD_TERMINATOR
#define HDI_CMD_SZ 2
#else
#define HDI_CMD_SZ 1
#endif

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */

static const lpuart_config_t uart_config = {
    .baudRate_Bps  = 115200,
    .dataBitsCount = kLPUART_EightDataBits,
    .parityMode    = kLPUART_ParityDisabled,
    .stopBitCount  = kLPUART_OneStopBit,
    .isMsb         = false,
    .enableTx      = true,
    .enableRx      = true,
};

static void HDI_sendCmd(uint8_t cmd)
{
    uint8_t hdi_cmd[HDI_CMD_SZ];
    hdi_cmd[0] = cmd;
#ifdef HDI_CMD_TERMINATOR
    hdi_cmd[1] = HDI_CMD_TERMINATOR;
#endif
    (void)HDI_WriteBlocking(hdi_cmd, HDI_CMD_SZ);
}
/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

int HDI_Init(void)
{
    static uint8_t initialized = 0U;

    if (initialized != 1U)
    {
        CLOCK_EnableClock(kCLOCK_Lpuart1);
        CLOCK_SetIpSrc(kCLOCK_Lpuart1, kCLOCK_IpSrcFro192M);
        (void)LPUART_Init(LPUART1, &uart_config, 32000000);

#if defined(HDI_TEST) && (HDI_TEST > 0)
        const char *test = "HDI Init\n";
        HDI_WriteBlocking((uint8_t *)test, strlen(test));
#endif

        initialized = 1U;
    }

    return 0;
}

int HDI_WriteBlocking(const uint8_t *data, size_t size)
{
    return (int)LPUART_WriteBlocking(LPUART1, data, size);
}

int HDI_ReadBlocking(uint8_t *out, size_t size)
{
    return (int)LPUART_ReadBlocking(LPUART1, out, size);
}

void HDI_Set_Mode_Ble(void)
{
    HDI_sendCmd(HDI_CMD_BLE);
}

void HDI_Set_Mode_Zigbee(void)
{
    HDI_sendCmd(HDI_CMD_ZB);
}

void HDI_Set_Mode_Gfsk(void)
{
    HDI_sendCmd(HDI_CMD_GFSK);
}

void HDI_Set_Mode_Initiator(void)
{
    HDI_sendCmd(HDI_CMD_CS_INITIATOR);
}

void HDI_Set_Mode_Reflector(void)
{
    HDI_sendCmd(HDI_CMD_CS_REFLECTOR);
}
