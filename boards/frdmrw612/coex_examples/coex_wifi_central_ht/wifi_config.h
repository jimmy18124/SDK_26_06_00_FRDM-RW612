/*
 * Copyright 2020-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#ifndef _WIFI_CONFIG_H_
#define _WIFI_CONFIG_H_

#define CONFIG_IPV6 1
#define CONFIG_MAX_IPV6_ADDRESSES 8

#if defined(RW610)
#define CONFIG_MAX_RESCAN_LIMIT 30
#define PRINTF_FLOAT_ENABLE 1
#if CONFIG_WIFI_BLE_COEX_APP
#define CONFIG_HOST_SLEEP 1
#define CONFIG_POWER_MANAGER 1
#endif
#define CONFIG_MEF_CFG 1
/** If define CONFIG_TX_RX_ZERO_COPY 1, please make sure
 *  #define PBUF_POOL_BUFSIZE 1752
 *  in lwipopts.h
 */
#define CONFIG_TX_RX_ZERO_COPY 1
#define CONFIG_ANT_DETECT 1
#define CONFIG_RF_TEST_MODE 1
#endif

/*
 * Config options for wpa supplicant
 */
#if CONFIG_WPA_SUPP_MBEDTLS
#define CONFIG_WPA_SUPP      1

#if !CONFIG_HOSTAPD
#undef CONFIG_HOSTAPD
#define CONFIG_HOSTAPD 1
#endif

#if CONFIG_WPA_SUPP
#ifdef RW610
#define CONFIG_WPA_SUPP_WPS               1
#define CONFIG_WPA_SUPP_CRYPTO_ENTERPRISE 1
#define CONFIG_WPA_SUPP_CRYPTO_AP_ENTERPRISE 1
#define CONFIG_WIFI_USB_FILE_ACCESS       0

#define CONFIG_WPA_SUPP_DPP 1

#if (CONFIG_WPA_SUPP_CRYPTO_ENTERPRISE || CONFIG_WPA_SUPP_CRYPTO_AP_ENTERPRISE)
#define CONFIG_EAP_TLS 1
#define CONFIG_EAP_PEAP 1
#define CONFIG_EAP_TTLS 1
#define CONFIG_EAP_FAST 1
#define CONFIG_EAP_SIM 1
#define CONFIG_EAP_AKA 1
#define CONFIG_EAP_AKA_PRIME 1

#if (CONFIG_EAP_PEAP || CONFIG_EAP_TTLS || CONFIG_EAP_FAST)
#define CONFIG_EAP_MSCHAPV2 1
#define CONFIG_EAP_GTC 1
#endif
#endif

#else
#define CONFIG_WPA_SUPP_WPS               1
#define CONFIG_WPA_SUPP_WPA3              1
#define CONFIG_WPA_SUPP_CRYPTO_ENTERPRISE 0

#if defined(SD9177)
#define CONFIG_WPA_SUPP_DPP 0

#if CONFIG_WPA_SUPP_DPP
#define CONFIG_WPA_SUPP_DPP2 1
#define CONFIG_WPA_SUPP_DPP3 1
#define CONFIG_RX_CHAN_INFO  1
#define CONFIG_TXPD_RXPD_V3  1
#endif
#endif

#if CONFIG_WPA_SUPP_CRYPTO_ENTERPRISE

#define CONFIG_WPA_SUPP_CRYPTO_AP_ENTERPRISE 1

#define CONFIG_EAP_TLS 1
#define CONFIG_EAP_PEAP 1
#define CONFIG_EAP_TTLS 1
#define CONFIG_EAP_FAST 1
#define CONFIG_EAP_SIM 1
#define CONFIG_EAP_AKA 1
#define CONFIG_EAP_AKA_PRIME 1
#define CONFIG_EAP_MSCHAPV2 1
#define CONFIG_EAP_GTC 1

#endif
#endif
#endif /* CONFIG_WPA_SUPP */
#else
#define CONFIG_WPA_SUPP_CRYPTO 0
#endif /* CONFIG_WPA_SUPP_MBEDTLS */

/* Logs */
#define CONFIG_ENABLE_ERROR_LOGS   1
#define CONFIG_ENABLE_WARNING_LOGS 1

/* WLCMGR debug */
#define CONFIG_WLCMGR_DEBUG        0

/*
 * Wifi extra debug options
 */
#define CONFIG_WIFI_EXTRA_DEBUG     0
#define CONFIG_WIFI_EVENTS_DEBUG    0
#define CONFIG_WIFI_CMD_RESP_DEBUG  0
#define CONFIG_WIFI_PKT_DEBUG       0
#define CONFIG_WIFI_SCAN_DEBUG      0
#define CONFIG_WIFI_IO_INFO_DUMP    0
#define CONFIG_WIFI_IO_DEBUG        0
#define CONFIG_WIFI_IO_DUMP         0
#define CONFIG_WIFI_MEM_DEBUG       0
#define CONFIG_WIFI_AMPDU_DEBUG     0
#define CONFIG_WIFI_TIMER_DEBUG     0
#define CONFIG_WIFI_SDIO_DEBUG      0
#define CONFIG_WIFI_FW_DEBUG        0
#define CONFIG_WIFI_UAP_DEBUG       0
#define CONFIG_WPS_DEBUG            0
#define CONFIG_FW_VDLL_DEBUG        0
#define CONFIG_DHCP_SERVER_DEBUG    0
#define CONFIG_WIFI_SDIO_DEBUG      0
#define CONFIG_FWDNLD_IO_DEBUG      0

/*
 * Heap debug options
 */
#define CONFIG_HEAP_DEBUG           0
#define CONFIG_HEAP_STAT            0

#endif /* _WIFI_CONFIG_H_ */
