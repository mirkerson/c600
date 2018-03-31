#ifndef _WIFI_H_
#define _WIFI_H_

#include "factory_api.h"

#define HOST_AP             0
#define STATION             1
#define INVALID_NET          -1


typedef enum
{
	ENC_WPA = 0,
	ENC_WEP,
	ENC_NONE,
	ENC_HIDE,

    ENC_UNVALID
} enc_ssid_t;

extern factory_func_callback *ffc_net;

#endif

