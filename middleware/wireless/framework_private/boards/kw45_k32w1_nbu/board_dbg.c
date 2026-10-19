/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*! *********************************************************************************
 * Include
 ********************************************************************************** */
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fwk_platform.h"
#include "rfmc_ctrl.h"
#include "board_dbg.h"

#if defined(gLoggingActive_d) && (gLoggingActive_d == 1)
#include "dbg_logging.h"
#else

/* if you got an error on dupplicated definition DbgLogAdd, turn gLoggingActive_d to 1 */
void DbgLogAdd(const char *function, const char *fmt, int n, ...)
{
    NOT_USED(function);
    NOT_USED(fmt);
    NOT_USED(n);
}
#endif

/*! *********************************************************************************
 * Private definitions
 ********************************************************************************** */
//#define NBU_DBG_ENABLE_RF_ACTIVE

#define BOARD_DBG_SHARED_LOG_BUF_SHARED_OFFSET 0x0

/* Currently need to apply an offset in the .logging section in linkerscript (section size is 0x200) */
#define BOARD_DBG_SHARED_LOG_BUF_SHARED_ADDR (0xb0000000 + (0x5480 + BOARD_DBG_SHARED_LOG_BUF_SHARED_OFFSET))
#define BOARD_DBG_SHARED_LOG_BUF_SHARED_SIZE (0x200)

/*! *********************************************************************************
 * Private type definitions
 ********************************************************************************** */

/*! *********************************************************************************
 * Private memory declarations
 ********************************************************************************** */
/* GPIOD pins used for debug */
#ifdef NBU_DBG_ENABLE_RF_ACTIVE
const uint32_t dbg_pins[] = {2, 3};
#else
const uint32_t dbg_pins[] = {1, 2, 3};
#endif
const uint32_t nb_dbg_pins = sizeof(dbg_pins) / sizeof(dbg_pins[0]);

/*! *********************************************************************************
 * Private functions
 ********************************************************************************** */

/*! *********************************************************************************
 * Public functions
 ********************************************************************************** */

void NbuDbg_DtestInit(void)
{
#if defined(gDbg_Enabled_d) && (gDbg_Enabled_d > 0) && defined(gDbgUseDtest) && (gDbgUseDtest > 0)
    RADIO_CTRL->DTEST_CTRL = (RADIO_CTRL->DTEST_CTRL & ~(RADIO_CTRL_DTEST_CTRL_DTEST_OUT_REG_EN_MASK | 0xff)) |
                             (RADIO_CTRL_DTEST_CTRL_DTEST_EN(1) | 0x43);

    CIU2->CIU2_TESTBUS_CTRL = CIU2_CIU2_TESTBUS_CTRL_TESTBUS_SEL(8);

    /* Signal bank select
     * 0x21:
     * DTEST0: tx_on_req
     * DTEST1: rx_on_req
     * DTEST2: tx_on
     * DTEST3: rx_on
     * Olivier's settings: 0x091A0A03 */
    BLE2_REG->BLE_REG_DEBUG_BANK_SEL = 0x01010221U; // rx/tx on/req on dtest[0:3]
    // BLE2_REG->BLE_REG_DEBUG_BANK_SEL = 0x01010B0BU;    // enable counter output on dtest[0:7]

    /* Select BLE testbus output */
    BTU2_REG->BTU_RIF_TESTBUS_SEL_0 = 0x5555U;
    BTU2_REG->BTU_RIF_TESTBUS_SEL_1 = 0x5555U;
    /* Testbus rotate */
    BTU2_REG->BTU_RIF_TESTBUS_CONTROL = 0x0U;
#endif
}

void NbuDbg_RfActiveInit(void)
{
#ifdef NBU_DBG_ENABLE_RF_ACTIVE
    gpio_pin_config_t config;
    config.pinDirection = kGPIO_DigitalOutput;
    config.outputLogic  = 0;

    PLATFORM_RemoteActiveReq();

    /* Configure PTD1 to output RF_GPO_4 */
    PORT_SetPinMux(PORTD, 1, kPORT_MuxAlt4);
    GPIO_PinInit(GPIOD, 1, &config);

    /* Configure RFMC_COEX to output RF_ACTIVE on RF_GPO_4 */
    RFMC->RF2P4GHZ_COEXT &= ~(RFMC_RF2P4GHZ_COEXT_RFGPO_SRC_MASK | RFMC_RF2P4GHZ_COEXT_RFGPO_OBE_MASK);
    RFMC->RF2P4GHZ_COEXT |= RFMC_RF2P4GHZ_COEXT_RFGPO_SRC(0x0U) | RFMC_RF2P4GHZ_COEXT_RFGPO_OBE(0x10U);

    PLATFORM_RemoteActiveRel();
#endif
}

void NbuDbg_IO_Init(void)
{
#if defined(gDbg_Enabled_d) && (gDbg_Enabled_d == 1) && defined(gDbgUseDbgIos) && (gDbgUseDbgIos > 0)
    gpio_pin_config_t config;
    config.pinDirection = kGPIO_DigitalOutput;
    config.outputLogic  = 0;

    /* Need to wake up SOC XBAR bus to access GPIOD */
    PLATFORM_RemoteActiveReq();

    for (uint32_t i = 0U; i < nb_dbg_pins; i++)
    {
        PORT_SetPinMux(PORTD, dbg_pins[i], kPORT_MuxAsGpio);
        GPIO_PinInit(GPIOD, dbg_pins[i], &config);
        /* Test toggle */
        GPIO_PinWrite(GPIOD, dbg_pins[i], 0U);
        GPIO_PinWrite(GPIOD, dbg_pins[i], 1U);
        GPIO_PinWrite(GPIOD, dbg_pins[i], 0U);
    }

    PLATFORM_RemoteActiveRel();
#endif
}

void NbuDbg_IO_Set(unsigned long io, unsigned short level)
{
#if defined(gDbg_Enabled_d) && (gDbg_Enabled_d == 1) && defined(gDbgUseDbgIos) && (gDbgUseDbgIos > 0)
    /* Need to wake up SOC XBAR bus to access GPIOD */
    /* Ideally disable lowpower on application core to avoid calling RemoteActiveReq/Rel as it
     *  is really intrusive if you call it */
    // PLATFORM_RemoteActiveReq();

    if (io < nb_dbg_pins)
    {
        //  TODO: fix it :D2 pin config is corrupted, need to reconfigure it here
        // PORT_SetPinMux(PORTD, dbg_pins[io], kPORT_MuxAsGpio);
        GPIO_PinWrite(GPIOD, dbg_pins[io], level);
    }

    // PLATFORM_RemoteActiveRel();
#endif
}

void NbuDbg_ConfigInit(bool_t first_time)
{
#if defined(gLoggingActive_d) && (gLoggingActive_d == 1)

#if defined(BOARD_DBG_SHARED_LOG_BUF_SHARED_ADDR)
    uint32_t dbg_ext_logging_start_2 = BOARD_DBG_SHARED_LOG_BUF_SHARED_ADDR;
    uint32_t dbg_ext_logging_size_2  = BOARD_DBG_SHARED_LOG_BUF_SHARED_SIZE;

    DbgLogInitExt((uint32_t)dbg_ext_logging_start_2, (size_t)dbg_ext_logging_size_2);
    DBG_DBG_LOG_EXT(DBG_DBG, "NBU start log addr=%x size=%x", dbg_ext_logging_start_2, dbg_ext_logging_size_2);

#else
    /* Definition defined in linker script */
    extern uint32_t dbg_ext_logging_start;
    extern uint32_t dbg_ext_logging_size;

    DbgLogInitExt((uint32_t)&dbg_ext_logging_start, (size_t)&dbg_ext_logging_size);
    DBG_DBG_LOG_EXT(DBG_DBG, "NBU start log addr=%x size=%x", &dbg_ext_logging_start, &dbg_ext_logging_size);

#endif

#endif
}

extern uint32_t _REG_HAL_GetClock(void);
uint32_t        Dbg_GetTimestamp(void)
{
    return _REG_HAL_GetClock();
}
