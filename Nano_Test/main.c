#include "FreeRTOS.h"
#include "task.h"
#include "app.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "nano_wifi.h"
#include "socket_task.h"

#define MAIN_TASK_PRIORITY    1U
#define MAIN_TASK_STACK_DEPTH 800U

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
    unsigned int channel;
    unsigned int acs_band;
    char softap_ip[16] = {0};

    (void)param;

    PRINTF("Nano_Test starting\r\n");
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

#if 0
    channel  = 0U;
    acs_band = WPL_AP_ACS_BAND_5GHZ;
    PRINTF("[WIFI] Starting SoftAP '%s', 5 GHz auto channel\r\n", APP_SOFTAP_SSID);
#else
    channel  = 36;
    acs_band = WPL_AP_ACS_BAND_5GHZ;
    PRINTF("[WIFI] Starting SoftAP '%s', fixed channel %u\r\n", APP_SOFTAP_SSID, channel);
#endif

    result = WPL_Start_AP(APP_SOFTAP_SSID, APP_SOFTAP_PASSWORD, (int)channel, acs_band);
    if (result != WPLRET_SUCCESS)
    {
        stop_on_error("WPL_Start_AP", result);
    }

    if (WPL_GetIP(softap_ip, 0) == WPLRET_SUCCESS)
    {
        PRINTF("[WIFI] SoftAP ready: SSID=%s IP=%s\r\n", APP_SOFTAP_SSID, softap_ip);
    }
    else
    {
        PRINTF("[WIFI] SoftAP ready, IP query failed\r\n");
    }

    if (socket_tasks_start() != 0)
    {
        PRINTF("[ERROR] Failed to create socket tasks\r\n");
        for (;;)
        {
            vTaskDelay(pdMS_TO_TICKS(1000U));
        }
    }

    vTaskDelete(NULL);
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
