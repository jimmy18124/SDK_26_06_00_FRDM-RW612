/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

/*${header:start}*/
#include "clock_config.h"
#include "board.h"
#include "pin_mux.h"
#include "fsl_debug_console.h"
#include "controller.h"
#include "fsl_power.h"
#include "usb_host_config.h"
#include "usb_host.h"
#include "fsl_device_registers.h"
#if (((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP)) || defined(RW612_SERIES))
#include "psa/crypto.h"
#endif /* CONFIG_BT_SMP */
#if(CONFIG_WIFI_BLE_COEX_APP)
#include "fsl_rtc.h"
#endif
/*${header:end}*/

/*${function:start}*/
#if (defined(DATA_SECTION_IS_CACHEABLE) && (DATA_SECTION_IS_CACHEABLE))
#define APP_FREERTOS_NONCACHE_HEAP_DATA __attribute__((section("freertos_nocache_heap, \"aw\", %nobits @")))
#define APP_FREERTOS_HEAP_NONINIT_DATA_ALIGN(n) __attribute__((aligned(n))) APP_FREERTOS_NONCACHE_HEAP_DATA
#endif

#if ((defined USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI))
#define CONTROLLER_ID kUSB_ControllerKhci0
#endif /* USB_HOST_CONFIG_KHCI */
#if ((defined USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI))
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif /* USB_HOST_CONFIG_EHCI */
#if ((defined USB_HOST_CONFIG_OHCI) && (USB_HOST_CONFIG_OHCI))
#define CONTROLLER_ID kUSB_ControllerOhci0
#endif /* USB_HOST_CONFIG_OHCI */
#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS))
#define CONTROLLER_ID kUSB_ControllerIp3516Hs0
#endif /* USB_HOST_CONFIG_IP3516HS */

#if defined(__GIC_PRIO_BITS)
#define USB_HOST_INTERRUPT_PRIORITY (25U)
#elif defined(__NVIC_PRIO_BITS) && (__NVIC_PRIO_BITS >= 3)
#define USB_HOST_INTERRUPT_PRIORITY (6U)
#else
#define USB_HOST_INTERRUPT_PRIORITY (3U)
#endif

#if(CONFIG_OT_CLI)
/* OpenThread UART config */
#define BOARD_OT_UART_FRG_CLK \
    (&(const clock_frg_clk_config_t){0, kCLOCK_FrgPllDiv, 255, 0}) /*!< Select FRG0 mux as frg_pll */
#define BOARD_OT_UART_CLK_ATTACH kFRG_to_FLEXCOMM0

static void BOARD_InitOtConsole(void)
{
    /* attach FRGx clock to FLEXCOMMx */
    CLOCK_SetFRGClock(BOARD_OT_UART_FRG_CLK);
    CLOCK_AttachClk(BOARD_OT_UART_CLK_ATTACH);
}
#endif

void BOARD_InitHardware(void)
{
    BOARD_InitBootPins();
#if defined(gAppLowPowerClockMode_d) && (gAppLowPowerClockMode_d > 0)
    BOARD_BootClockLPR();
#else
    BOARD_InitBootClocks();
#endif
#if defined(gBoardUseFro32k_d) && (gBoardUseFro32k_d > 0)
    CLOCK_AttachClk(kRC32K_to_CLK32K);
#else
    CLOCK_EnableXtal32K(true);
    CLOCK_AttachClk(kXTAL32K_to_CLK32K);
#endif
    BOARD_InitDebugConsole();
    BOARD_InitSleepPinConfig();
#if(CONFIG_OT_CLI)
    BOARD_InitOtConsole();
#endif

#if (((defined(CONFIG_BT_SMP)) && (CONFIG_BT_SMP)) || defined(RW612_SERIES))
    psa_crypto_init();
#endif /* CONFIG_BT_SMP */
#if(CONFIG_WIFI_BLE_COEX_APP)
    RTC_Init(RTC);
#endif
}

void USB_HostClockInit(void)
{
    /* reset USB */
    RESET_PeripheralReset(kUSB_RST_SHIFT_RSTn);
    /* enable usb clock */
    CLOCK_EnableClock(kCLOCK_Usb);
    /* enable usb phy clock */
    CLOCK_EnableUsbhsPhyClock();
}

void USB_HostIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbHOSTEhciIrq[] = USBHS_IRQS;
    irqNumber                = usbHOSTEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];
    /* USB_HOST_CONFIG_EHCI */

    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}
/*${function:end}*/
