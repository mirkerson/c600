#ifndef _AD_TYPE_H_
#define _AD_TYPE_H_

//#include "g_para.h"
//#include "common_def.h"
#include "factory_api.h"


#define MAX_BUFF_LEN 4*1024*1024
#define MAX_TONE_BUF_LEN 1*1024*1024

#define IS_PLAY  0x10
#define IS_PAUSE 0x20
#define IS_STOP  0x30
#define IS_DOWNLOAD  0x40
#define IS_BUFFERING  0x50


#define BUFFER_PACKET_NUM 512
#define CACHE_PACKET_NUM 200

#define AOInf_CODEC 0
#define AOInf_I2S 1
#define AOInf_RESET 0xff



enum AD_CMD{
    AD_PLAY = 0, 
    AD_PAUSE,    
    AD_STOP,
    AD_SEEK,
    AD_READY_GO_NEXT,
    AD_AUX_START,
    AD_AUX_STOP,
        
    AD_GET_STATUS,

  #ifdef __MULTI_ROOM__  
    MR_SERV_EXIT,
  #endif
    AD_PLAY_BUFFER,
    AD_RECORD,
    AD_STOP_RECORD,  
    AD_GET_METADATA,
    AD_GET_BUFFER_STA,
    AD_CHANGE_SPEED,
    
    AD_CMD_MAX
};

extern factory_func_callback ffc_inst;

#endif

