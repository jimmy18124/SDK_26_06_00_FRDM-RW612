#ifndef _APP_H_
#define _APP_H_

#define APP_SOFTAP_SSID                 "Nano_Test"
#define APP_SOFTAP_PASSWORD             "12345678"
#define APP_SOFTAP_IP                   "192.168.167.1"
#define APP_SOFTAP_MAX_CLIENTS          2U

/* Select the high-rate data path. TCP remains connected in both modes. */
#define APP_DATA_TRANSPORT_TCP          1U
#define APP_DATA_TRANSPORT_UDP          2U
#define APP_DATA_TRANSPORT              APP_DATA_TRANSPORT_UDP

#if ((APP_DATA_TRANSPORT != APP_DATA_TRANSPORT_TCP) && \
     (APP_DATA_TRANSPORT != APP_DATA_TRANSPORT_UDP))
#error "APP_DATA_TRANSPORT must select TCP or UDP"
#endif

/* The peer must use this static IP, or be guaranteed the first DHCP lease. */
#define APP_TCP_SERVER_IP               "192.168.167.2"
#define APP_TCP_SERVER_PORT             15151U
#define APP_TCP_KEEPALIVE_IDLE_SEC      5
#define APP_TCP_KEEPALIVE_INTERVAL_SEC  1
#define APP_TCP_KEEPALIVE_COUNT         5

#define APP_UDP_PEER_IP                 "192.168.167.2"
#define APP_UDP_PEER_PORT               15153U
#define APP_UDP_LOCAL_PORT              15153U

#ifdef __cplusplus
extern "C"{
#endif

void BOARD_InitHardware(void);

#ifdef __cplusplus
}
#endif
#endif /* _APP_H_ */
