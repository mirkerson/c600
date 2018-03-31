
#ifndef _API_H_
#define _API_H_

#include <semaphore.h>
#include <stdbool.h>

#include "common_def.h"
#include "hw.h"
#include "factory_api.h"

#define BT     0x55
#define AUX    0x66

typedef struct {
     void (*play)(const char* uri);
     void (*pause)(void);
     void (*stop)(void);
     void (*next)(void);
     void (*previous)(void);
     void (*setNextUri)(const char* uri);
     void (*seek)(int position);
     int (*getDuration)(void);
     int (*getPosition)(void);
     void (*setMute)(bool);
     bool (*getMute)(void);
     void (*setVolume)(int volume);
     int (*getVolume)(void);
     void (*setPlaymode)(int playmode);
     int (*getPlaymode)(void);
     int (*getState)(void);
     int (*rotate)(int degree);
     int (*scale)(int ratio);
     int (*setVideoSizeMode)(int mode);
     void (*setNetwork)(const char* ssid, const char* key);
     void (*initialize)();
     void (*fetchFmt)(int bit, int rate, int channel);
}PlayerStruct;

enum {
    PLAY = 0,
    PAUSE,
    NEXT,
    PREVIOUS,
    VOLUP,
    VOLDOWN

};

typedef struct  
{  
    vtable *vptr;  
    void (*open_service)(void);
    void (*close_service)(void);
    void (*into_dormancy)(void);
    void (*outof_dormancy)(void);
    int  (*commit)(int cmd);
    
    int fa_sta;
    sem_t mp_sem_sk;
   
    PlayerStruct *airplay;
    PlayerStruct *dlna;

}mpush;

typedef struct  
{  
    vtable *vptr;  
    int (*is_pause)(void);   
    int (*clear_sta)(void);      

}mbl;

typedef struct  
{  
    vtable *vptr;  
    void (*insert)(void); 
    void (*unplug)(void); 
    int (*check_next)(void);
    void (*go_play_next)(void);
    void (*go_play_prev)(void);

}mlocal;

typedef struct  
{  
    vtable *vptr;  
    int (*check_next)(void);
    void (*go_play_next)(void);
    void (*go_play_prev)(void);
    void (*release_res)(void);

}mifly;

typedef struct  
{  
    vtable *vptr;  
    int (*check_next)(void);
    void (*go_play_next)(void);
    void (*go_play_prev)(void);    
    int (*url_parse)(char *url);
    void (*release_res)(void);    
    char **radio_src;
    int   ch;

}msimple;

typedef struct  
{  
    vtable *vptr;  
    void (*open_service)(void);
    void (*close_service)(void);
    void (*into_dormancy)(void);
    void (*outof_dormancy)(void);
    int  (*simple_config)(void);
    int  (*sendmsg)(char* method , char* name);    
    int (*check_next)(void);
    void (*go_play_next)(void);
    int  (*analyze_pcm)(void);
    int (*notify_app)(void);

}mali;



typedef struct {
   mpush   *mpobj;
   mbl     *mblobj;
   mlocal  *mlcobj;
   mifly   *miflyobj;
   msimple *msimpleobj;
   mali    *maliobj;   
}MODE_INST;
extern MODE_INST mi;

typedef struct {
    void (*new_object)(MODE_INST* m, int mode);
    void (*del_object)(MODE_INST* m, int mode);
    int cur_mode;
}FIREAIR;
extern FIREAIR fa_obj;


#define XI_MAX  7

#define IS_PLAY  0x10
#define IS_PAUSE 0x20
#define IS_STOP  0x30
#define IS_DOWNLOAD  0x40
#define IS_BUFFERING  0x50

#define AOInf_CODEC 0
#define AOInf_I2S 1
#define AOInf_RESET 0xff

#define HOST_AP             0
#define STATION             1
#define INVALID_NET          -1

//extern void set_offline_state(int sta);

//extern int wifi_interrpt_flag; 
extern factory_func_callback ffc_inst;
    
#endif

