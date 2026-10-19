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
#include "cli.h"
#include "coex_cli.h"
#if CONFIG_COEX_ENABLE_WIFI_MENU
#include "wlan.h"
#endif
#if CONFIG_COEX_ENABLE_PM_MENU
#include "coex_lpm.h"
#endif

#define CMD_BUF_SIZE 512
char cmd_buf[CMD_BUF_SIZE];

void coex_menuPrint(void);

/*******************************************************************************
 * Code
 ******************************************************************************/

static void printSeparator(void)
{
    PRINTF("========================================\r\n");
}

static struct cli_command built_ins[] = {
    {"help (exit: 0)", NULL, help_command},
};

int coex_cli_init()
{
    if (cli_register_commands(&built_ins[0], (int)(sizeof(built_ins) / sizeof(struct cli_command))) != 0)
    {
        return -1;
    }
#if CONFIG_COEX_ENABLE_PM_MENU
    if (!pm_cli_init())
    {
        (void)PRINTF("Failed to initialize power manager cli.\r\n");
        return -1;
    }
#endif

    return 0;
}

static int coex_console_input(void)
{
    char a;
    int i;

retry:
    i = 0;
    PRINTF("\n>");

    while (1)
    {
        a = pollChar();
        if (a == '\n')
            continue;
        if (a == '\r')
        {
            cmd_buf[i]='\0';
            break;
        }

        cmd_buf[i++] = a;

        if (i >= CMD_BUF_SIZE)
        {
            PRINTF("Error: overflow!\r\n");
            goto retry;
        }
    }

    if (strcmp(cmd_buf, "0")==0)
    {
        coex_menuPrint();
        return -1;
    }
    if (strcmp(cmd_buf,"")==0)
    {
        goto retry;
    }

    return 0;
}

/*!
 * @brief Function to start MAIN BT MENU.
 */
#if CONFIG_COEX_ENABLE_BLE_MENU
extern int appl_main (int argc, char **argv);
static void bt_menu(void)
{
    appl_main(0 , NULL);
}
#endif

/*!
 * @brief Function to start zigbee menu.
 */
#if CONFIG_COEX_ENABLE_ZIGBEE_MENU
extern void vProcessCommand(char *tmp);
static void zigbee_menu(void)
{
    int ret = 0;

    PRINTF("Enter Zigbee commands\n");

    while (1)
    {
        ret = coex_console_input();
        if (ret == -1)
        {
            return;
        }

        PRINTF("zigbee commands: %s\n", cmd_buf);
        vProcessCommand(cmd_buf);
    }
}
#endif

#if CONFIG_COEX_ENABLE_WIFI_MENU

static void execute_commnd(char *wlan_command, int argv, char** argc)
{
    osa_status_t status;
    (void)status;
    const struct cli_command *command = NULL;
    const char *p;
    int i=0;

    i = ((p = strchr(wlan_command, '.')) == NULL) ? 0 : (p - wlan_command);
    command = lookup_command(wlan_command, i);
    if (command == NULL)
        PRINTF("Invalid Command");
    else
        command->function(argv, argc);
}

void wifi_cli(void)
{
    char* cmd_list[20] = {0};
    char *wlantemp = NULL;
    char *psave = NULL;
    char *delim = " ";
    int j = 1;
    int ret = 0;

    PRINTF("Enter WiFi commands\n");

    while (1)
    {
        ret = coex_console_input();
        if (ret == -1)
        {
            return;
        }

        wlantemp = strtok_r(cmd_buf, delim, &psave);
        j = 1;
        cmd_list[0] = cmd_buf;

        while(1)
        {
            wlantemp = strtok_r(NULL, delim, &psave);

            if(wlantemp == NULL)
                break;

            cmd_list[j++]=wlantemp;
        }

        PRINTF("wifi commands: %s\n", cmd_buf);
        execute_commnd(cmd_buf, j, cmd_list);
    }
}
#endif

#if CONFIG_COEX_ENABLE_PM_MENU
static void pm_menu(void)
{   
    char* cmd_list[20] = {0};
    char *temp = NULL;
    char *psave = NULL;
    char *delim = " ";
    int j = 1;
    int ret = 0;
    struct pm_command *pm_cmd = NULL;

    PRINTF("Enter Power manager commands\n");

    while (1)
    {
        ret = coex_console_input();
        if (ret == -1)
        {
            return;
        }

        temp = strtok_r(cmd_buf, delim, &psave);
        j = 1;
        cmd_list[0] = cmd_buf;

        while(1)
        {
            temp = strtok_r(NULL, delim, &psave);

            if(temp == NULL)
                break;

            cmd_list[j++]=temp;
        }

        pm_cmd = looup_pm_cmd(cmd_list[0], strlen(cmd_list[0]));
        if(pm_cmd != NULL)
        {
            PRINTF("power manager commands: %s\n", cmd_list[0]);
            pm_cmd->function(j, cmd_list);

        }
        else
        {
            PRINTF("Invaild pm command.\n");
            PRINTF("Enter help to check supported pm commands.\n");
        }
    }
}
#endif

/*!
 * @brief struct for coex menu.
 */
typedef struct
{
    char key;
    void (*func)(void);
    const char *text;
} menu_item_t;

menu_item_t menuItems[] = {
    {'0',coex_menuPrint, "Coex menu print"},
#if(CONFIG_COEX_ENABLE_ZIGBEE_MENU)
    {'z', zigbee_menu, "Zigbee MAIN MENU"},
#endif
#if CONFIG_COEX_ENABLE_BLE_MENU
    {'b', bt_menu, "BT MAIN MENU"},
#endif
#if CONFIG_COEX_ENABLE_WIFI_MENU
    {'w',wifi_cli, "Wifi Cli"},
#endif
#if CONFIG_COEX_ENABLE_PM_MENU
    {'p',pm_menu, "Power manager MAIN MENU"},
#endif
    {0, NULL, NULL},
};

/*!
 * @brief Function to coex menu print.
 */
void coex_menuPrint(void)
{
    PRINTF(" Coex menu called\r\n");
    printSeparator();

    for (int i = 0; menuItems[i].text != NULL; i++)
    {
        if (menuItems[i].key)
            PRINTF("  %c  %s\r\n", menuItems[i].key, menuItems[i].text);
        else
            PRINTF("  %d  %s\r\n", i, menuItems[i].text);
    }

    printSeparator();
}

/*!
 * @brief Function to coex menu action.
 */
void coex_menuAction(int ch)
{
    if (ch == '\r' || ch == ' ' || ch == '\n')
    {
        printSeparator();
        return;
    }

    for (int i = 0; menuItems[i].func != NULL; i++)
    {
        if (menuItems[i].key == ch)
        {
            PRINTF("Key '%c': %s\r\n", ch, menuItems[i].text);
            menuItems[i].func();
            return;
        }
    }
    PRINTF("ERROR: No action bound to '%c'\r\n", ch);
}


/*!
 * @brief Function to Poll char.
 */
int pollChar(void)
{
    int tmp;
    tmp = GETCHAR();
    return tmp;
}

//*********************************************************************************************