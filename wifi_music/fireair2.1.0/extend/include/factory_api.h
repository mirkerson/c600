
#ifndef _FACTORY_API_H_
#define _FACTORY_API_H_

#include "common_def.h"
#include "ota_api.h"
#include "driver_api.h"
#include "network_api.h"
#include "player_api.h"


typedef struct  
{  
	int  (*check)(update_info *info);
	int  (*download)(update_info *info);
	void (*burn)(void);
	
}b_ota_update;

typedef struct  
{  
	int  (*adplayer)(int cmd, void *arg);
	void (*get_file_context)(s_file_context *fc);
	void (*set_file_context)(s_file_context *fc);
	void (*tone_play)(void* arg, int type);	
	void (*select_ad)(void);	
    int  (*get_ad)(void);
     
}b_ad_player;

typedef struct  
{  
	int   (*connect)(char *encryptType, char *ssid, char *pwd);
	int   (*disconnect)(void);
	int   (*get_status)(void);
	char* (*get_ip)(char *e_name);
	int   (*get_mode)(void);
	void  (*start_ap)(char *ap_name,char *ap_pwd, int flag);
	void  (*stop_ap)(void);
	int   (*init)(void);

}b_net_connect;

typedef struct  
{  
	int (*led_init)(void);
	int (*set_status)(int status);

}b_dev_driver;


typedef struct  
{  
    b_ota_update    updator;
    b_ad_player     player;
    b_net_connect   network;
    b_dev_driver    driver;

}factory_func_callback;


#endif

