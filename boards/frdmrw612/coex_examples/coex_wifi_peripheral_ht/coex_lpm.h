/*
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

#ifndef __COEX_LPM_H__
#define __COEX_LPM_H__

/***********************************************************************************************************************
 * Structs
 **********************************************************************************************************************/

 #define PM_MAX_COMMANDS 5

/** Structure for registering CLI commands */
struct pm_command
{
    /** The name of the CLI command */
    const char *name;
    /** The help text associated with the command */
    const char *help;
    /** The function that should be invoked for this command. */
    void (*function)(int argc, char **argv);
};

typedef struct
{
    struct pm_command *commands[PM_MAX_COMMANDS];
    unsigned int num_commands;

} pm_cli;

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/
struct pm_command *looup_pm_cmd(char *name, int len);
bool pm_cli_init(void);
void APP_LPTimerCreate(void);

#endif