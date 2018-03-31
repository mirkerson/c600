
#ifndef _MPUSH_H_
#define _MPUSH_H_

#include <semaphore.h>
#include <sys/types.h>
#include "fireair_lib.h"
#include "factory_api.h"

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

typedef struct mutex_sem_lock {
    //sem_t g_sem_sk;//seek
    //sem_t g_sem_ps;//play and stop
    //sem_t g_sem_dec;//play and stop
    
    pthread_mutex_t alsa_control_lock;
    pthread_mutex_t mp_mutex_lock;
    //pthread_mutex_t mp_ps_lock;
    
} s_ms_lock;


extern vtable pushmode;

#endif

