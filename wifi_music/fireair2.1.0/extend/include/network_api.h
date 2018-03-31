
#ifndef _NETWORK_API_H_
#define _NETWORK_API_H_

extern int staConnect(char *encryptType, char *ssid, char *pwd);
extern int staDisConnect();
extern int getWifiMode(void);
extern int check_network_status(void);
extern int wifi_init(void);
extern void startAp(char *ap_name,char *ap_pwd, int flag);
extern void stopAp();

extern void net_set_external_callback(void *ffc);

#endif

