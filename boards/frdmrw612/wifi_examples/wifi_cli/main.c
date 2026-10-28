/** @file main.c
 *
 *  @brief main file
 *
 *  Copyright 2020 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

// SDK Included Files
#include "board.h"
#include "fsl_debug_console.h"
#include "wlan_bt_fw.h"
#include "wlan.h"
#include "wifi.h"
#include "wm_net.h"
#include <osa.h>
#if CONFIG_NXP_WIFI_SOFTAP_SUPPORT
#include "dhcp-server.h"
#endif
#include "cli.h"
#include "wifi_ping.h"
#include "iperf.h"
#include "app.h"
#ifndef RW610
#include "wifi_bt_config.h"
#else
#include "fsl_power.h"
#include "fsl_ocotp.h"
#endif
#include "cli_utils.h"
#if CONFIG_HOST_SLEEP
#include "host_sleep.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
int wlan_driver_init(void);
#ifndef RW610
#if CONFIG_HOST_SLEEP
int wlan_hs_cli_init(void);
#endif
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

#if CONFIG_WPS2
#define MAIN_TASK_STACK_SIZE 6000
#else
#define MAIN_TASK_STACK_SIZE 4096
#endif

static void task_main(osa_task_param_t arg);

static OSA_TASK_DEFINE(task_main, WLAN_TASK_PRI_LOW, 1, MAIN_TASK_STACK_SIZE, 0);

OSA_TASK_HANDLE_DEFINE(task_main_Handle);

static void printSeparator(void)
{
    (void)PRINTF("========================================\r\n");
}

/* Callback Function passed to WLAN Connection Manager. The callback function
 * gets called when there are WLAN Events that need to be handled by the
 * application.
 */
int wlan_event_callback(enum wlan_event_reason reason, void *data)
{
    int ret;
    struct wlan_ip_config addr;
    char ssid[IEEEtypes_SSID_SIZE + 1] = {0};
    char ip[16];
    static int auth_fail = 0;
#if CONFIG_NXP_WIFI_SOFTAP_SUPPORT
    wlan_uap_client_disassoc_t *disassoc_resp = data;
#endif
    int i;

    switch (reason)
    {
        /* WLAN 连接管理器初始化成功，已经可以执行扫描、连接和注册 CLI 命令。 */
        case WLAN_REASON_INITIALIZED:
            (void)PRINTF("app_cb: WLAN initialized\r\n");
            printSeparator();

            ret = wlan_basic_cli_init();
            if (ret != WM_SUCCESS)
            {
                (void)PRINTF("Failed to initialize BASIC WLAN CLIs\r\n");
                return 0;
            }

            ret = wlan_cli_init();
            if (ret != WM_SUCCESS)
            {
                (void)PRINTF("Failed to initialize WLAN CLIs\r\n");
                return 0;
            }
            (void)PRINTF("WLAN CLIs are initialized\r\n");
            printSeparator();

            ret = wlan_enhanced_cli_init();
            if (ret != WM_SUCCESS)
            {
                (void)PRINTF("Failed to initialize WLAN CLIs\r\n");
                return 0;
            }
            (void)PRINTF("ENHANCED WLAN CLIs are initialized\r\n");
            printSeparator();
#ifdef RW610
#if CONFIG_HOST_SLEEP
            ret = host_sleep_cli_init();
            if (ret != WM_SUCCESS)
            {
                (void)PRINTF("Failed to initialize WLAN CLIs\r\n");
                return 0;
            }
            (void)PRINTF("HOST SLEEP CLIs are initialized\r\n");
            printSeparator();
#endif
#endif
            ret = ping_cli_init();
            if (ret != WM_SUCCESS)
            {
                (void)PRINTF("Failed to initialize PING CLI\r\n");
                return 0;
            }

            ret = iperf_cli_init();
            if (ret != WM_SUCCESS)
            {
                (void)PRINTF("Failed to initialize IPERF CLI\r\n");
                return 0;
            }

#if CONFIG_NXP_WIFI_SOFTAP_SUPPORT
            ret = dhcpd_cli_init();
            if (ret != WM_SUCCESS)
            {
                (void)PRINTF("Failed to initialize DHCP Server CLI\r\n");
                return 0;
            }
#endif

            (void)PRINTF("CLIs Available:\r\n");
            printSeparator();
            help_command(0, NULL);
            printSeparator();
            break;
        /* WLAN 连接管理器初始化失败，当前不能执行扫描或连接操作。 */
        case WLAN_REASON_INITIALIZATION_FAILED:
            (void)PRINTF("app_cb: WLAN: initialization failed\r\n");
            break;
        /* Station 已完成 802.11 认证，当前状态为 WLAN_ASSOCIATED，尚未表示已经获得 IP。 */
        case WLAN_REASON_AUTH_SUCCESS:
            (void)PRINTF("app_cb: WLAN: authenticated to network\r\n");
            break;
        /* Station 已成功连接到 AP；随后读取当前 SSID、IPv4 地址和有效的 IPv6 地址。 */
        case WLAN_REASON_SUCCESS:
            (void)PRINTF("app_cb: WLAN: connected to network\r\n");
            ret = wlan_get_address(&addr);
            if (ret != WM_SUCCESS)
            {
                (void)PRINTF("failed to get IP address\r\n");
                return 0;
            }

            net_inet_ntoa(addr.ipv4.address, ip);

            ret = wlan_get_current_network_ssid(ssid);
            if (ret != WM_SUCCESS)
            {
                (void)PRINTF("Failed to get External AP network\r\n");
                return 0;
            }

            (void)PRINTF("Connected to following BSS:\r\n");
            (void)PRINTF("SSID = [%s]\r\n", ssid);
            if (addr.ipv4.address != 0U)
            {
                (void)PRINTF("IPv4 Address: [%s]\r\n", ip);
            }
#if CONFIG_IPV6
            for (i = 0; i < CONFIG_MAX_IPV6_ADDRESSES; i++)
            {
                if (ip6_addr_isvalid(((int8_t)(addr.ipv6[i].addr_state))) != 0)
                {
                    (void)PRINTF("IPv6 Address: %-13s:\t%s (%s)\r\n",
                                 ipv6_addr_type_to_desc((struct net_ipv6_config *)&addr.ipv6[i]),
                                 inet6_ntoa(addr.ipv6[i].address), ipv6_addr_state_to_desc(addr.ipv6[i].addr_state));
                }
            }
            (void)PRINTF("\r\n");
#endif
            auth_fail = 0;
            break;
        /* 连接流程失败，但不是明确的“找不到网络”或“认证失败”原因。 */
        case WLAN_REASON_CONNECT_FAILED:
            (void)PRINTF("app_cb: WLAN: connect failed\r\n");
            break;
        /* 扫描结果中没有找到正在连接的 SSID/BSSID，连接管理器回到断开状态。 */
        case WLAN_REASON_NETWORK_NOT_FOUND:
            (void)PRINTF("app_cb: WLAN: network not found\r\n");
            break;
        /* AP 认证失败，常见原因是密码错误或安全配置不匹配；连续失败三次后主动断开。 */
        case WLAN_REASON_NETWORK_AUTH_FAILED:
            (void)PRINTF("app_cb: WLAN: network authentication failed\r\n");
            auth_fail++;
            if (auth_fail >= 3)
            {
                (void)PRINTF("Authentication Failed. Disconnecting ... \r\n");
                (void)wlan_disconnect();
                auth_fail = 0;
            }
            break;
        /* DHCP 成功获得或续租 IPv4 地址；此时网络地址配置已经完成。 */
        case WLAN_REASON_ADDRESS_SUCCESS:
            (void)PRINTF("network mgr: DHCP new lease\r\n");
            break;
        /* 未能获得 IP 地址，或 TCP/IP 地址配置失败/因 DHCP 错误丢失。 */
        case WLAN_REASON_ADDRESS_FAILED:
            (void)PRINTF("app_cb: failed to obtain an IP address\r\n");
            break;
        /* 用户主动断开，或取消了正在进行的连接操作；不是无线链路自然丢失。 */
        case WLAN_REASON_USER_DISCONNECT:
            (void)PRINTF("app_cb: disconnected\r\n");
            auth_fail = 0;
            break;
        /* 已连接的 Station 丢失与当前 AP 的无线链路，data 可能携带 802.11 断开原因码。 */
        case WLAN_REASON_LINK_LOST:
            (void)PRINTF("app_cb: WLAN: link lost\r\n");
            break;
        /* 收到 AP 的信道切换通告，当前网络即将或已经切换到新的信道。 */
        case WLAN_REASON_CHAN_SWITCH:
            (void)PRINTF("app_cb: WLAN: channel switch\r\n");
            break;
        /* WPS 网络断开，或 WPS 连接过程被取消；当前回调不做额外处理。 */
        case WLAN_REASON_WPS_DISCONNECT:
            break;
#if CONFIG_NXP_WIFI_SOFTAP_SUPPORT
        /* SoftAP/UAP 启动成功；本示例随后启动 DHCP Server，为接入客户端分配地址。 */
        case WLAN_REASON_UAP_SUCCESS:
            (void)PRINTF("app_cb: WLAN: UAP Started\r\n");
            ret = wlan_get_current_uap_network_ssid(ssid);

            if (ret != WM_SUCCESS)
            {
                (void)PRINTF("Failed to get Soft AP network\r\n");
                return 0;
            }

            printSeparator();
            (void)PRINTF("Soft AP \"%s\" started successfully\r\n", ssid);
            printSeparator();
            ret = dhcp_server_start(net_get_uap_handle());
            if (ret != 0)
            {
                PRINTF("%s\r\n", dhcp_server_err_str(ret));
            }
            else
            {
                (void)PRINTF("DHCP Server started successfully\r\n");
            }
            printSeparator();
            break;
        /* 客户端已经关联到 SoftAP 的 BSS；data 指向客户端 MAC 地址。 */
        case WLAN_REASON_UAP_CLIENT_ASSOC:
            (void)PRINTF("app_cb: WLAN: UAP a Client Associated\r\n");
            printSeparator();
            (void)PRINTF("Client => ");
            print_mac((const char *)data);
            (void)PRINTF("Associated with Soft AP\r\n");
            printSeparator();
            break;
        /* 客户端已完成认证并连接到 SoftAP；data 指向客户端 MAC 地址。 */
        case WLAN_REASON_UAP_CLIENT_CONN:
            (void)PRINTF("app_cb: WLAN: UAP a Client Connected\r\n");
            printSeparator();
            (void)PRINTF("Client => ");
            print_mac((const char *)data);
            (void)PRINTF("Connected with Soft AP\r\n");
            printSeparator();
            break;
        /* 客户端离开或被取消关联；data 指向 wlan_uap_client_disassoc_t，包含 MAC 和 reason_code。 */
        case WLAN_REASON_UAP_CLIENT_DISSOC:
            printSeparator();
            (void)PRINTF("app_cb: WLAN: UAP a Client Dissociated:");
            (void)PRINTF(" Client MAC => ");
            print_mac((const char *)(disassoc_resp->sta_addr));
            (void)PRINTF(" Reason code => ");
            (void)PRINTF("%d\r\n", disassoc_resp->reason_code);
            printSeparator();
            break;
        /* SoftAP/UAP 启动失败；当前回调不做额外处理。 */
        case WLAN_REASON_UAP_START_FAILED:
            break;
        /* SoftAP/UAP 停止失败；当前回调不做额外处理。 */
        case WLAN_REASON_UAP_STOP_FAILED:
            break;
        /* SoftAP/UAP 已停止；本示例同时停止 SoftAP 使用的 DHCP Server。 */
        case WLAN_REASON_UAP_STOPPED:
            (void)PRINTF("app_cb: WLAN: UAP Stopped\r\n");
            printSeparator();
            (void)PRINTF("Soft AP stopped successfully\r\n");
            printSeparator();

            dhcp_server_stop();

            (void)PRINTF("DHCP Server stopped successfully\r\n");
            printSeparator();
            break;
#endif /* CONFIG_NXP_WIFI_SOFTAP_SUPPORT */
        /* WLAN 进入省电模式；data 在底层可能表示 IEEE、Deep Sleep 或 WNM 省电类型。 */
        case WLAN_REASON_PS_ENTER:
            break;
        /* WLAN 退出省电模式；data 在底层可能表示对应的省电类型。 */
        case WLAN_REASON_PS_EXIT:
            break;
        /* Station 的 RSSI 低于订阅阈值；开启漫游时可能由连接管理器内部处理。 */
        case WLAN_REASON_RSSI_LOW:
#if CONFIG_SUBSCRIBE_EVENT_SUPPORT
        /* Station 的 RSSI 高于订阅阈值。 */
        case WLAN_REASON_RSSI_HIGH:
        /* Station 的 SNR 低于订阅阈值。 */
        case WLAN_REASON_SNR_LOW:
        /* Station 的 SNR 高于订阅阈值。 */
        case WLAN_REASON_SNR_HIGH:
        /* Station 达到订阅配置的最大失败次数。 */
        case WLAN_REASON_MAX_FAIL:
        /* Station 在订阅周期内未收到足够的 Beacon。 */
        case WLAN_REASON_BEACON_MISSED:
        /* 数据包接收 RSSI 低于订阅阈值。 */
        case WLAN_REASON_DATA_RSSI_LOW:
        /* 数据包接收 RSSI 高于订阅阈值。 */
        case WLAN_REASON_DATA_RSSI_HIGH:
        /* 数据包接收 SNR 低于订阅阈值。 */
        case WLAN_REASON_DATA_SNR_LOW:
        /* 数据包接收 SNR 高于订阅阈值。 */
        case WLAN_REASON_DATA_SNR_HIGH:
        /* 链路质量达到订阅条件，可能涉及 SNR、速率或发送延迟。 */
        case WLAN_REASON_LINK_QUALITY:
        /* Beacon 即将丢失，表示无线链路质量正在恶化。 */
        case WLAN_REASON_PRE_BEACON_LOST:
#endif
            break;
        /* Wi-Fi 固件进入挂起/卡死状态；本分支当前只打印事件，不执行复位。 */
        case WLAN_REASON_FW_HANG:
        /* Wi-Fi 固件已经完成复位；与 FW_HANG 共用处理分支，仅打印事件编号。 */
        case WLAN_REASON_FW_RESET:
#ifdef RW610
            (void)PRINTF("app_cb: WLAN: FW hang Event: %d\r\n", reason);
#endif
            break;
        default:
            (void)PRINTF("app_cb: WLAN: Unknown Event: %d\r\n", reason);
            break;
    }
    return 0;
}

int wlan_driver_init(void)
{
    int result = 0;

#if CONFIG_FW_DNLD_ASYNC
    result = wlan_init_nb(wlan_fw_bin, wlan_fw_bin_len, wlan_event_callback);
#else
    /* Initialize WIFI Driver */
    result = wlan_init(wlan_fw_bin, wlan_fw_bin_len);

    assert(0 == result);

    result = wlan_start(wlan_event_callback);

    assert(0 == result);
#endif
    return result;
}

#ifndef RW610
#if CONFIG_HOST_SLEEP
static void test_mcu_suspend(int argc, char **argv)
{
    (void)mcu_suspend();
}

static struct cli_command hs_commands[] = {
    {"mcu-suspend", NULL, test_mcu_suspend},
};

int wlan_hs_cli_init(void)
{
    unsigned int i;

    for (i = 0; i < sizeof(hs_commands) / sizeof(struct cli_command); i++)
    {
        if (cli_register_command(&hs_commands[i]) != 0)
        {
            return -1;
        }
    }

    return 0;
}
#endif
#endif

static void task_main(osa_task_param_t arg)
{
    int32_t result = 0;
    (void)result;

    (void)PRINTF("Initialize CLI\r\n");
    printSeparator();

    result = cli_init();

    assert(WM_SUCCESS == result);

#if CONFIG_HOST_SLEEP
#ifndef RW610
    hostsleep_init(wlan_hs_pre_cfg, wlan_hs_post_cfg);
#else
    (void)hostsleep_init();
#endif
#endif

    (void)PRINTF("Initialize WLAN Driver\r\n");
    printSeparator();

    /* Initialize WIFI Driver */
    result = wlan_driver_init();

    assert(WM_SUCCESS == result);

#ifndef RW610
#if CONFIG_HOST_SLEEP
    result = wlan_hs_cli_init();

    assert(WM_SUCCESS == result);
#endif
#endif

#if CONFIG_FW_DNLD_ASYNC
    wlan_nb_task_cleanup();
#endif

    while (true)
    {
        /* wait for interface up */
        OSA_TimeDelay(5000);
    }
}

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
int main(void)
{
    OSA_Init();

    BOARD_InitHardware();
#ifdef RW610
    POWER_PowerOffBle();
#endif

    printSeparator();
    (void)PRINTF("Nano RW612 Test\r\n");
    printSeparator();

    (void)OSA_TaskCreate((osa_task_handle_t)task_main_Handle, OSA_TASK(task_main), NULL);

    OSA_Start();

    return 0;
}
