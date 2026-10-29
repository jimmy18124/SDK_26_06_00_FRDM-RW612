#include "FreeRTOS.h"
#include "task.h"
#include "app.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "nano_wifi.h"
#include "socket_task.h"
#include "wlan.h"

#define MAIN_TASK_PRIORITY    1U
#define MAIN_TASK_STACK_DEPTH 800U
#define SOFTAP_MONITOR_INTERVAL_MS 1000U
#define SOFTAP_RETRY_DELAY_MS      2000U

enum
{
    FREERTOS_FAULT_NONE = 0U,
    FREERTOS_FAULT_STACK_OVERFLOW,
    FREERTOS_FAULT_MALLOC_FAILED
};

volatile uint32_t g_freertos_fault_code = FREERTOS_FAULT_NONE;
TaskHandle_t volatile g_freertos_stack_overflow_task = NULL;
const char *volatile g_freertos_stack_overflow_task_name = NULL;

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
    g_freertos_stack_overflow_task      = task;
    g_freertos_stack_overflow_task_name = task_name;
    g_freertos_fault_code               = FREERTOS_FAULT_STACK_OVERFLOW;
    taskDISABLE_INTERRUPTS();
    for (;;)
    {
    }
}

void vApplicationMallocFailedHook(void)
{
    g_freertos_fault_code = FREERTOS_FAULT_MALLOC_FAILED;
    taskDISABLE_INTERRUPTS();
    for (;;)
    {
    }
}

static void print_separator(void)
{
    PRINTF("========================================\r\n");
}

static void wifi_link_status_callback(bool link_state)
{
    PRINTF("[WIFI] STA link %s\r\n", link_state ? "restored" : "lost");
}

static void stop_on_error(const char *operation, wpl_ret_t error)
{
    PRINTF("[ERROR] %s failed: %d\r\n", operation, (int)error);
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000U));
    }
}

static void main_task(void *param)
{
    wpl_ret_t result;
    int max_clients_result;
    unsigned int channel;
    unsigned int acs_band;
    bool socket_tasks_started = false;
    bool ap_cleanup_needed = false;

    (void)param;

    PRINTF("Nano_Test starting C5\r\n");
    PRINTF("Build time: %s %s\r\n", __DATE__, __TIME__);
    print_separator();

    result = WPL_Init();
    if (result != WPLRET_SUCCESS)
    {
        stop_on_error("WPL_Init", result);
    }

    result = WPL_Start(wifi_link_status_callback);
    if (result != WPLRET_SUCCESS)
    {
        stop_on_error("WPL_Start", result);
    }

    max_clients_result = wlan_set_uap_max_clients(APP_SOFTAP_MAX_CLIENTS);
    if (max_clients_result != WM_SUCCESS)
    {
        PRINTF("[ERROR] Failed to set SoftAP max clients: %d\r\n", max_clients_result);
        for (;;)
        {
            vTaskDelay(pdMS_TO_TICKS(1000U));
        }
    }
    PRINTF("[WIFI] SoftAP max clients: %u\r\n", (unsigned int)APP_SOFTAP_MAX_CLIENTS);

    wlan_version_extended();
    
#if 1
    channel  = 0U;
    acs_band = WPL_AP_ACS_BAND_5GHZ;
    PRINTF("[WIFI] Starting SoftAP '%s', 5 GHz auto channel\r\n", APP_SOFTAP_SSID);
#else
    channel  = 36;
    acs_band = WPL_AP_ACS_BAND_5GHZ;
    PRINTF("[WIFI] Starting SoftAP '%s', fixed channel %u\r\n", APP_SOFTAP_SSID, channel);
#endif

    for (;;)
    {
        if (!WPL_Is_AP_Active())
        {
            char softap_ip[16] = {0};

            if (ap_cleanup_needed)
            {
                (void)WPL_Prepare_AP_Restart();
            }

            result = WPL_Start_AP(APP_SOFTAP_SSID, APP_SOFTAP_PASSWORD, (int)channel, acs_band);
            ap_cleanup_needed = true;
            if (result != WPLRET_SUCCESS)
            {
                PRINTF("[WIFI] SoftAP start failed: %d; retrying in %u ms\r\n",
                       (int)result, (unsigned int)SOFTAP_RETRY_DELAY_MS);
                vTaskDelay(pdMS_TO_TICKS(SOFTAP_RETRY_DELAY_MS));
                continue;
            }

            if (WPL_GetIP(softap_ip, 0) == WPLRET_SUCCESS)
            {
                PRINTF("[WIFI] SoftAP ready: SSID=%s IP=%s\r\n", APP_SOFTAP_SSID, softap_ip);
            }
            else
            {
                PRINTF("[WIFI] SoftAP ready, IP query failed\r\n");
            }
        }

        if (!socket_tasks_started)
        {
            if (socket_tasks_start() != 0)
            {
                PRINTF("[ERROR] Failed to create socket tasks; retrying in %u ms\r\n",
                       (unsigned int)SOFTAP_RETRY_DELAY_MS);
                vTaskDelay(pdMS_TO_TICKS(SOFTAP_RETRY_DELAY_MS));
                continue;
            }
            socket_tasks_started = true;
        }

        vTaskDelay(pdMS_TO_TICKS(SOFTAP_MONITOR_INTERVAL_MS));
    }
}

int main(void)
{
    BaseType_t result;

    BOARD_InitHardware();
    print_separator();

    result = xTaskCreate(main_task, "main", MAIN_TASK_STACK_DEPTH, NULL, MAIN_TASK_PRIORITY, NULL);
    assert(result == pdPASS);

    vTaskStartScheduler(); 

    for (;;)
    {
    }
}
