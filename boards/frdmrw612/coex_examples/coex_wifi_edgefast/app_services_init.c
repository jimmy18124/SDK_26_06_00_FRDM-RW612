/*-----------------------------------------------------------------------------
* Copyright 2024 NXP
*
* SPDX-License-Identifier: BSD-3-Clause
* The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
*-----------------------------------------------------------------------------*/

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */

#include "board.h"

#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
#include "PWR_Interface.h"
#include "board_lp.h"
#include "fsl_component_serial_manager.h"
#endif

#if defined(APP_USE_SENSORS) && (APP_USE_SENSORS > 0)
#include "sensors.h"
#endif

/* -------------------------------------------------------------------------- */
/*                               Private memory                               */
/* -------------------------------------------------------------------------- */

#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
static const serial_manager_lowpower_critical_CBs_t gSerMgr_LowpowerCriticalCBs = {
    .serialEnterLowpowerCriticalFunc = &PWR_LowPowerEnterCritical,
    .serialExitLowpowerCriticalFunc  = &PWR_LowPowerExitCritical,
};

#define LOWPOWER_DEFAULT_ENABLE_DURATION_MS (10000U)

static PWR_LowpowerMode_t g_currentMode = PWR_WFI;
static TimerHandle_t      lpTimer       = NULL;

extern bool pm_state;
extern void APP_ConfigureNextLowPowerMode(uint8_t nextMode, uint32_t timeMs);
#endif

/* -------------------------------------------------------------------------- */
/*                              Public Variables                              */
/* -------------------------------------------------------------------------- */

#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
SERIAL_MANAGER_HANDLE_DEFINE(gSerMgrIf);
#endif

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
static void APP_ServiceInitLowpower(void)
{
    PWR_ReturnStatus_t status = PWR_Success;

    /* It is required to initialize PWR module so the application
     * can call PWR API during its init (wake up sources...) */
    PWR_Init();

    /* Initialize board_lp module, likely to register the enter/exit
     * low power callback to Power Manager */
    BOARD_LowPowerInit();

    /* Set WFI constraint by default (works for All application)
     * Application will be allowed to release the WFI constraint and set a deepest lowpower mode constraint such as
     * DeepSleep or PowerDown if it needs more optimization */
    status = PWR_SetLowPowerModeConstraint(PWR_WFI);
    assert(status == PWR_Success);
    (void)status;

    /* Register PWR functions into SerialManager module in order to disable device lowpower
        during SerialManager processing. Typically, allow only WFI instruction when
        uart data are processed by serail manager  */
    SerialManager_SetLowpowerCriticalCb(&gSerMgr_LowpowerCriticalCBs);
}

static void APP_LPTimerCallback(TimerHandle_t timer_h)
{
    (void)timer_h;

    APP_ConfigureNextLowPowerMode(PWR_WFI, 0U);
    pm_state = false;
}

static void APP_LPTimerCreate(void)
{
    static bool once = false;
    if (!once)
    {
        /* Create FreeRTOS timer which will be used to disable low power after a specific time */
        lpTimer = xTimerCreate("LP timer", (TickType_t)(LOWPOWER_DEFAULT_ENABLE_DURATION_MS / portTICK_PERIOD_MS),
                               pdFALSE, NULL, &APP_LPTimerCallback);
        assert(lpTimer != NULL);
        once = true;
    }
}
#endif /* APP_LOWPOWER_ENABLED */

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void APP_InitServices(void)
{
#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
    APP_ServiceInitLowpower();

    APP_LPTimerCreate();
#endif

#if defined(APP_USE_SENSORS) && (APP_USE_SENSORS > 0)
    SENSORS_InitAdc();
#endif /* APP_USE_SENSORS */
}

#if defined(APP_LOWPOWER_ENABLED) && (APP_LOWPOWER_ENABLED > 0)
void APP_ConfigureNextLowPowerMode(uint8_t nextMode, uint32_t timeMs)
{
    (void)PWR_ReleaseLowPowerModeConstraint(g_currentMode); /* MISRA CID 26556646 */
    (void)PWR_SetLowPowerModeConstraint((PWR_LowpowerMode_t)nextMode);
    g_currentMode = nextMode;

    if (timeMs > 0U)
    {
        /* Update timer duration */
        (void)xTimerChangePeriod(lpTimer, timeMs / portTICK_PERIOD_MS, 0);

        /* Start the timer, during this time, the configured low power mode will be used as much as possible by
         * the system, when the timer expires, the low power mode will be limited to WFI until next command
         * This is to make sure the serial interface becomes available again to the user */
        if (xTimerStart(lpTimer, 0) != pdPASS)
        {
            assert(0);
        }
    }
}
#endif