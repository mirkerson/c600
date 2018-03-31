#ifndef SMART_VOICE_COMMON_H
#define SMART_VOICE_COMMON_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
//#include "smart_asr.h"


#define CPU_CODEC_SND           0
#define I2S_SND                 1

#define HOTPLUG_INSERT            1
#define HOTPLUG_PULL_OUT          0



#define STATION_CONNECT_OK    1
#define STATION_CONNECT_FAIL  0


//#define MULTI_AUDIO
//#define SIBICHI
#define  SMART_ASR_DEMO

#define  ALI_WIFI_CONFIG        3
#define  AP_STA_WIFI_CONFIG     2
#define  OTHERS_WIFI_CONFIG     1
#define  NO_WIFI_CONFIG         0 


#if 0
#define  WIFI_SIMPLECONFIG_CONFIG  0 //rtl8188
#define  WIFI_AP_STATION_CONFIG    1
#define  WIFI_RS_CONFIG            2
#define  WIFI_ALI_SIMPLE_CONFIG    3
#define  WIFI_ALI_SOFTAP_CONFIG    4
#define  WIFI_CONFIG               1
#endif

#define  RANG_LOW  20
#define  RANG_HIGH 9000


#define    SMART_RECORD_ASR_ALI    0
#define    SMART_RECORD_ASR_IFLY   1
#define    SMART_RECORD_RS_WIFI    2
#define    SMART_RECORD_AISPEECH   3
#define    SMART_RECORD_OTHER      4  


enum WIFI_STATUS{
   WIFI_CONNECTED,
   WIFI_CONNECTING,
   WIFI_DISCONNECT,

};

#ifdef LOG_TAG
#define LOG_DEBUG(fmt...)   	\
	    do {			\
	        printf("[D/%s] %s,line:%d: ", LOG_TAG, __FUNCTION__, __LINE__);\
	        printf(fmt);\
	        printf("\n");\
	    }while(0);	
#else
#define LOG_DEBUG(fmt, args...)
#endif


#define FREE_MEM(ptr)        if(ptr){free((void *)ptr); ptr = NULL;}
#define FREE_JSON_OBJ(ptr)   if(ptr){json_object_put(ptr); ptr = NULL;}
#define FREE_JSON_FRAME(ptr) if(ptr){    \
        FREE_JSON_OBJ(ptr->pRootObj);    \
        FREE_MEM(ptr); ptr = NULL;}




//int get_sys_run_sec(void);
//unsigned long long get_time_us(void);
//unsigned int get_time_ms(void);
int user_update_smt_file(char *ssid,char * password,char * type);
int user_file_exist(const char *name);


#endif 
