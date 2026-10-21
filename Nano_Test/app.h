#ifndef _APP_H_
#define _APP_H_

#define APP_SOFTAP_SSID                 "Nano_Test"
#define APP_SOFTAP_PASSWORD             "12345678"
#define APP_SOFTAP_IP                   "192.168.167.1"



#define APP_TCP_SERVER_IP               "192.168.167.2"
#define APP_TCP_SERVER_PORT             15151U
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
