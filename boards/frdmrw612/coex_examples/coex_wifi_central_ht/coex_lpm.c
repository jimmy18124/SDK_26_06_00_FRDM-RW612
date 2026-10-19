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

extern void APP_SetTicklessIdle(bool enable);

#define LOWPOWER_DEFAULT_ENABLE_DURATION_MS (10000U)

bool pm_state = false;
static PWR_LowpowerMode_t g_currentMode = PWR_WFI;
static TimerHandle_t      lpTimer       = NULL;

static pm_cli pm_cmd_cli;

static void APP_ConfigureNextLowPowerMode(uint8_t nextMode, uint32_t timeMs)
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

/* Power manager commands.*/

static void pm_enable_cmd_usage(void)
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("enable <constraint> <duration>\r\n");
    (void)PRINTF("       <constraint> low power constraint, range: 1 ~ 3.\r\n");
    (void)PRINTF("       <duration> Duration of entering low power.\r\n");
    (void)PRINTF("                  Unit: milliseconds. \r\n");
    (void)PRINTF("                  Default value: 0xffffffffU\r\n");
    (void)PRINTF("For example:\r\n");
    (void)PRINTF("  enable 3 3000\r\n");
    (void)PRINTF("  Set low pwoer constraint as PM3.\r\n");
    (void)PRINTF("  The duration of entering lower power is 3 seconds.\r\n");
}

static void pm_enable_cmd(int argc, char **argv)
{
    uint8_t constraint = 0U;
    uint32_t time_ms = 0xffffffffUL;

    if (argc > 3)
    {
        PRINTF("COEX PM command: Invaild paramter numbers.\r\n");
        return;
    }

    if (pm_state == false)
    {
        constraint = atoi(argv[1]);
        if (argc == 3)
        {
            time_ms = atoi(argv[2]);
        }

        APP_ConfigureNextLowPowerMode(constraint, time_ms);
        APP_SetTicklessIdle(true);
        PRINTF("Constrain is PM%d. Ready to enter low power loop for %d ms\r\n", constraint, time_ms);

        pm_state = true;
    }

    return;
}

static void pm_help_cmd(int argc, char **argv)
{
    unsigned int i = 0, n = 0;

    (void)PRINTF("\r\n");
    while (i < PM_MAX_COMMANDS && n < pm_cmd_cli.num_commands)
    {
        if (pm_cmd_cli.commands[i]->name != NULL)
        {
            (void)PRINTF("%s %s\r\n", pm_cmd_cli.commands[i]->name,
                         pm_cmd_cli.commands[i]->help != NULL ? pm_cmd_cli.commands[i]->help : "");
            n++;
        }
        i++;
    }
}

static bool pm_cli_register_command(struct pm_command *command)
{
    unsigned int i;
    if (command->name == NULL || command->function == NULL)
    {
        return false;
    }

    if (pm_cmd_cli.num_commands < PM_MAX_COMMANDS)
    {
        /* Check if the command has already been registered.
         * Return true, if it has been registered.
         */
        for (i = 0; i < pm_cmd_cli.num_commands; i++)
        {
            if (pm_cmd_cli.commands[i] == command)
            {
                return true;
            }
        }
        pm_cmd_cli.commands[pm_cmd_cli.num_commands++] = command;
        return true;
    }

    return false;
}

static bool pm_cli_register_commands(struct pm_command *commands, int num_commands)
{
    int i;
    for (i = 0; i < num_commands; i++)
    {
        if (!pm_cli_register_command(commands++))
        {
            return false;
        }
    }
    return true;
}

struct pm_command *looup_pm_cmd(char *name, int len)
{
    unsigned int i = 0;
    unsigned int n = 0;

    while (i < PM_MAX_COMMANDS && n < pm_cmd_cli.num_commands)
    {
        if (pm_cmd_cli.commands[i]->name == NULL)
        {
            i++;
            continue;
        }
        /* See if partial or full match is expected */
        if (len != 0)
        {
            if (strncmp(pm_cmd_cli.commands[i]->name, name, (size_t)len) == 0)
            {
                return pm_cmd_cli.commands[i];
            }
        }
        else
        {
            if (strcmp(pm_cmd_cli.commands[i]->name, name) == 0)
            {
                return pm_cmd_cli.commands[i];
            }
        }

        i++;
        n++;
    }

    return NULL;
}

static struct pm_command pm_tests[] = {
    {"help", NULL, pm_help_cmd},
    {"enable", "<mode> <duration>", pm_enable_cmd},
};

bool pm_cli_init(void)
{
    return pm_cli_register_commands(pm_tests, (int)(sizeof(pm_tests) / sizeof(struct pm_command)));
}

/* Power manager timer */

static void APP_LPTimerCallback(TimerHandle_t timer_h)
{
    (void)timer_h;

    APP_ConfigureNextLowPowerMode(PWR_WFI, 0U);

    APP_SetTicklessIdle(false);
    pm_state = false;

    PRINTF("Exit low power loop.\r\n");
}

void APP_LPTimerCreate(void)
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