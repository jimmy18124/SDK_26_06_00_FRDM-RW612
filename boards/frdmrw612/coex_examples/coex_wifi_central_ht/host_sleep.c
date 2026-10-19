/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

 ///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include "fsl_debug_console.h"
#include "coex_lpm.h"
#include "PWR_Interface.h"
#include "board_lp.h"
#include "fsl_component_serial_manager.h"
#include "fsl_pm_core.h"
#include "fsl_power.h"
#include "fsl_pm_device.h"
#include "board.h"
#include "host_sleep.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

#if CONFIG_POWER_MANAGER

status_t powerManager_PmNotify(pm_event_type_t eventType, uint8_t powerState, void *data);
AT_ALWAYS_ON_DATA_INIT(pm_notify_element_t coex_pm_notify) = {
    .notifyCallback = powerManager_PmNotify,
    .data           = NULL,
};
AT_ALWAYS_ON_DATA(pm_wakeup_source_t wlanWakeupSource);

extern int wakeup_by;
extern void wifi_print_wakeup_reason(uint16_t hs_wakeup_reason);
#define WAKEUP_BY_WLAN   0x1

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

void coex_LowPowerInit(void)
{
    status_t status;

    memset(&wlanWakeupSource, 0x0, sizeof(pm_wakeup_source_t));
	PM_InitWakeupSource(&wlanWakeupSource, WL_MCI_WAKEUP0_IRQn, NULL, true);

    status = PM_RegisterNotify(kPM_NotifyGroup1, &coex_pm_notify);
    assert(status == kStatus_Success);

    (void)status;
}

static void coex_dump_wakeup_source(void)
{
    if (wakeup_by == WAKEUP_BY_WLAN)
    {
        wifi_print_wakeup_reason(0);
    }
}

static void coex_sleep_pre_cfg(int mode)
{
    POWER_ClearWakeupStatus(WL_MCI_WAKEUP0_IRQn);
    POWER_EnableWakeup(WL_MCI_WAKEUP0_IRQn);
    //(void)PRINTF("Enter low power mode PM%d\r\n", mode);
}

static void coex_sleep_post_cfg(int mode)
{
    coex_dump_wakeup_source();
    //(void)PRINTF("Exit low power mode\r\n");
}

status_t powerManager_PmNotify(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if (eventType == kPM_EventEnteringSleep)
    {
        if (powerState >= 2)
        {
            coex_sleep_pre_cfg(powerState);
        }
    }
    else if (eventType == kPM_EventExitingSleep)
    {
        if (powerState >= 2)
        {
            coex_sleep_post_cfg(powerState);
        }
    }

    return kStatus_PMSuccess;
}

#endif