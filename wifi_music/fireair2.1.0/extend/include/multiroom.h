
#ifndef _MULTIROOM_H_
#define _MULTIROOM_H_

/*********************
    for multi-room 
***********************/
#ifdef __MULTI_ROOM__
//#define __UDP_SEARCH_DEV__
#ifndef __UDP_SEARCH_DEV__
#define __ANROID_APP_LINK__
#endif
#endif

#define MAX_DEV 8

//type
enum {
    ACT_SYNC_PLAY_TIME = 11,
    ACT_ADD_IN_PLAYING,
    ACT_PLAYER_START,
    ACT_PLAYER_STOP,
    ACT_PLAYER_PAUSE,
    ACT_PLAYER_BUFFERING,
    ACT_PLAYER_SEEK,
    ACT_PLAYER_UPDATE_VOLUME,
    ACT_PTHREAD_START,

    ACT_SYNC_OVER,
    
    ACT_SYNC_END
};

typedef struct  
{  
    void (*open_service)(void);
    void (*close_service)(void);
    int  (*get_volume)(void);
    void (*set_volume)(int vol);
	void (*stop)(void);

}multi_callback;

extern void* app_connection_creat_pthread(void);

#endif
