


#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

#include "common_def.h"
#include "mpush.h"
#include "fireair_lib.h"
#include "ad_type.h"
#include "rc.h"
#include "multiroom.h"

static int fa_state = UPNP_PLAYBACK_STATE_STOP;//协议回调状态

s_ms_lock ms_lk;

static int b_ServiceStarted = 0;

static mpush *mp_obj;

#ifdef __MULTI_ROOM__
static multi_callback mt_cb;
#endif

s_fa_context fac = 
{
    "fireair",
    "53357210",
    "55555555",
    "62900026",
    {0x3e, 0x0b, 0x32, 0x1f, 0xa8, 0xc9, 0xa1, 0xd8, 0x5a,
								0xef, 0x10, 0xc4, 0xda,  0x3d, 0x3c, 0x34, 0},
	NULL,
	NULL,
	NULL
};

void open_service_func(void);
void close_service_func(void);
int reverse_event_handle(int cmd);

#ifdef __MULTI_ROOM__
int multi_getVolume(void);
void multi_setVolume(int vol);
void multi_stop(void);
extern void multi_serv_exit(void);
#endif


extern int rc_commit(int cmd);

static int push_sleep = 0;

//#define AIRPLAY_IS_PLAYING() \
//( IsAirplayPlaying(fac.airplay_manager) && !push_sleep)

#define DLNA_CAN_NOT_PLAY() \
( push_sleep || IsAirplayPlaying(fac.airplay_manager) )


void push_into_dormancy(void)
{
    push_sleep = 1;
}

void push_outof_dormancy(void)
{
    push_sleep = 0;
}

mpush* push_init(void)
{
   
    mp_obj = (mpush *)malloc(sizeof(mpush));
    if(NULL == mp_obj)
    {
        return NULL;
    }
        
    mp_obj->vptr =  &pushmode; 
    mp_obj->open_service =  open_service_func; //open dlna and airplay service
    mp_obj->close_service =  close_service_func;
    mp_obj->into_dormancy =  push_into_dormancy; //push mode  into sleep
    mp_obj->outof_dormancy =  push_outof_dormancy;
    mp_obj->commit = reverse_event_handle;  //c600 notice app
    
#ifdef __MULTI_ROOM__

    mt_cb.open_service = open_service_func;
    mt_cb.close_service = close_service_func;
    mt_cb.get_volume = multi_getVolume;
    mt_cb.set_volume = multi_setVolume;
    mt_cb.stop = multi_stop;

    set_multi_callback(&mt_cb);
#endif

    //mp_obj->ctl_sta = IS_STOP;
    mp_obj->fa_sta = UPNP_PLAYBACK_STATE_STOP;

    pthread_mutex_init(&(ms_lk.mp_mutex_lock), NULL);
    pthread_mutex_init(&(ms_lk.alsa_control_lock), NULL);
    
	sem_init(&mp_obj->mp_sem_sk, 0, 1);
    
    mp_obj->airplay = &player_airplay;
    mp_obj->dlna = &player_dlna;
    
    mp_obj->vptr->init();    

    push_sleep = 1;

    return mp_obj;
    
}


void push_exit(void)
{
    pthread_mutex_destroy(&(ms_lk.mp_mutex_lock));
    pthread_mutex_destroy(&(ms_lk.alsa_control_lock));
    
    sem_destroy(&mp_obj->mp_sem_sk);

    mp_obj->vptr->unit();

    //close_service();
    
    free(mp_obj);
    mp_obj = NULL;    
    

}

#ifdef __MULTI_ROOM__

void multi_setVolume(int vol)
{
    int new_vol;

    if( DLNA_CAN_NOT_PLAY() )
        return;
    
    pthread_mutex_lock(&(ms_lk.alsa_control_lock));

    if(AOInf_I2S == ffc_inst.player.get_ad())
    {
        new_vol = floor(vol*1.13); 
        if(0 != new_vol)
            new_vol += 142;
        
        if(OK != audio_snd_ctl(1, "hw:1,0", "Master Volume", &new_vol))
            LOG_PRT("error occur!");
    }

    else
    {
        new_vol = ceil(vol*0.62);

        if(OK != audio_snd_ctl(1, "default", "head phone volume", &new_vol))
            LOG_PRT("error occur!");
    }

    pthread_mutex_unlock(&(ms_lk.alsa_control_lock));

}

int multi_getVolume(void)
{
	int old_vol, new_vol = 0;

    if( DLNA_CAN_NOT_PLAY() )
        return;
    
    pthread_mutex_lock(&(ms_lk.alsa_control_lock));
	
    if(AOInf_I2S == ffc_inst.player.get_ad())
    {
        if(OK == audio_snd_ctl(0, "hw:1,0", "Master Volume", &old_vol))
        {
            old_vol -= 142;
            if(old_vol < 0)
                old_vol = 0;
            new_vol = ceil(old_vol/1.13);
        }        
    }
    else
    {
        if(OK == audio_snd_ctl(0, "default", "head phone volume", &old_vol))
        {
            new_vol = floor(old_vol/0.62);
        }
    }
    
	pthread_mutex_unlock(&(ms_lk.alsa_control_lock));

	return new_vol;
}

void multi_stop(void)
{
    if( DLNA_CAN_NOT_PLAY() || (IS_STOP == mp_obj->vptr->get_status()) )
        return;

    pthread_mutex_lock(&(ms_lk.alsa_control_lock));

    mp_obj->vptr->stop();
    dlna_setstate(UPNP_PLAYBACK_STATE_STOP);
    
    pthread_mutex_unlock(&(ms_lk.alsa_control_lock));    
}

#endif

/********************************************************************************************************************************/
/********************************************************************************************************************************/
/********************************************************************************************************************************/

void dlna_setstate(int sta)
{
    fa_state = sta;
}

void dlna_play(const char* uri)
{
    int ret;
    
    LOG_PRT("%d %d\n", DLNA_CAN_NOT_PLAY(), mp_obj->vptr->get_status());

    if( DLNA_CAN_NOT_PLAY() || (IS_PLAY == mp_obj->vptr->get_status()) || (IS_DOWNLOAD == mp_obj->vptr->get_status())  
        || (IS_BUFFERING == mp_obj->vptr->get_status()) )
        return;
    
    pthread_mutex_lock(&(ms_lk.mp_mutex_lock));

#ifdef __MULTI_ROOM__
    s_file_context fc;    
    mp_obj->vptr->get_file_attr(&fc);   
    memset(fc.url, 0, sizeof(fc.url));
    strcpy(fc.url, uri);
    mp_obj->vptr->set_file_attr(&fc); //set uri into file context 

    server_notice_msg_handle(ACT_PLAYER_START); //notice clients to start play
#endif 

    ret = mp_obj->vptr->play(uri);
    if(OK == ret)
    {
    #ifdef __MULTI_ROOM__
        server_notice_msg_handle(ACT_PTHREAD_START); //notice clients to start playing 
    #endif    
        dlna_setstate(UPNP_PLAYBACK_STATE_PLAY);     
    }
    else
    {
    #ifdef __MULTI_ROOM__
        server_notice_msg_handle(ACT_PLAYER_STOP); //notice clients to stop 
    #endif
        
    }

    
    pthread_mutex_unlock(&(ms_lk.mp_mutex_lock));

}

void dlna_pause(void)
{
    if( DLNA_CAN_NOT_PLAY() ||  (IS_STOP == mp_obj->vptr->get_status()) || (IS_PAUSE == mp_obj->vptr->get_status())   )
        return;

#ifdef __MULTI_ROOM__
    server_notice_msg_handle(ACT_PLAYER_PAUSE);//notice clients to pause 
#endif

    mp_obj->vptr->pause();
    dlna_setstate(UPNP_PLAYBACK_STATE_PAUSE);
}

void dlna_stop(void)
{
    LOG_PRT();

    if( DLNA_CAN_NOT_PLAY() || (IS_STOP == mp_obj->vptr->get_status()) )
        return;

    pthread_mutex_lock(&(ms_lk.mp_mutex_lock));

#ifdef __MULTI_ROOM__
    server_notice_msg_handle(ACT_PLAYER_STOP); //notice clients to stop 
#endif

    mp_obj->vptr->stop();
    dlna_setstate(UPNP_PLAYBACK_STATE_STOP);
    
    pthread_mutex_unlock(&(ms_lk.mp_mutex_lock));    
}

void dlna_next(void)
{
    //useless
}

void dlna_previous(void)
{
    //useless
}

void dlna_setNextUri(const char* uri)
{
    //useless
}

void dlna_seek(int position)
{
    if( DLNA_CAN_NOT_PLAY() )
        return;

    s_file_context fc;    
    mp_obj->vptr->get_file_attr(&fc);   
    fc.seek_time = position/1000;   
    mp_obj->vptr->set_file_attr(&fc);  //set the seek time to file context
    
    alarm(1); //1s later, send a SIGALRM signal to sig_handler()

}

int dlna_getDuration(void)
{
    if( DLNA_CAN_NOT_PLAY() )
        return 0;

    s_file_context fc;
    
    mp_obj->vptr->get_file_attr(&fc);
    
    return (fc.duaration * 1000);
}

int dlna_getPosition(void)
{
    if( DLNA_CAN_NOT_PLAY() )
        return 0;

    s_file_context fc;
    
    mp_obj->vptr->get_file_attr(&fc);
    
    if(fc.postion == fc.duaration)
        dlna_setstate(UPNP_PLAYBACK_STATE_STOP);
    
    if(fc.seek_time > 0)
        fc.postion = fc.seek_time;
    
    return (fc.postion * 1000);

}

void dlna_setMute(bool mute)
{
}

bool dlna_getMute(void)
{
	return false;
}

void dlna_setVolume(int vol)
{
    int new_vol;
    
    if( DLNA_CAN_NOT_PLAY() )
        return;

    pthread_mutex_lock(&(ms_lk.mp_mutex_lock));

    if(AOInf_I2S == ffc_inst.player.get_ad())
    {
        new_vol = floor(vol*1.13); 
        if(0 != new_vol)
            new_vol += 142;
        
        if(OK != audio_snd_ctl(1, "hw:1,0", "Master Volume", &new_vol))
            LOG_PRT("error occur!");
    }
    else
    {
        new_vol = ceil(vol*0.62);

        if(OK != audio_snd_ctl(1, "default", "head phone volume", &new_vol))
            LOG_PRT("error occur!");
    }

#ifdef __MULTI_ROOM__
    server_notice_msg_handle(ACT_PLAYER_UPDATE_VOLUME);  //notice clients to update volume   
#endif    

    pthread_mutex_unlock(&(ms_lk.mp_mutex_lock));


}

int dlna_getVolume(void)
{
	int old_vol, new_vol = 0;

    if( DLNA_CAN_NOT_PLAY()  || (IS_STOP == mp_obj->vptr->get_status()) )
	    return 0;
        	
    pthread_mutex_lock(&(ms_lk.mp_mutex_lock));
	
    if(AOInf_I2S == ffc_inst.player.get_ad())
    {
        if(OK == audio_snd_ctl(0, "hw:1,0", "Master Volume", &old_vol))
        {
            old_vol -= 142;
            if(old_vol < 0)
                old_vol = 0;
            new_vol = ceil(old_vol/1.13);
        }        
    }
    else  
    {
        if(OK == audio_snd_ctl(0, "default", "head phone volume", &old_vol))
        {
            new_vol = floor(old_vol/0.62);
        }
    }
    
	pthread_mutex_unlock(&(ms_lk.mp_mutex_lock));

	return new_vol;
}


void dlna_setPlaymode(int mode)
{

}


int dlna_getPlayMode(void)
{
	return 0;
}

int dlna_getState(void)
{
    if( DLNA_CAN_NOT_PLAY() )
        return UPNP_PLAYBACK_STATE_STOP;

    if( (IS_DOWNLOAD == mp_obj->vptr->get_status()) || (IS_BUFFERING == mp_obj->vptr->get_status()) )
        dlna_setstate(UPNP_PLAYBACK_STATE_TRANSITION);
    else if(IS_PLAY == mp_obj->vptr->get_status())
        dlna_setstate(UPNP_PLAYBACK_STATE_PLAY);

    return fa_state;

}

int dlna_rotate(int degree)
{

	return 0;
}

int dlna_scale(int ratio)
{

	return 0;
}

int dlna_setVideoSizeMode(int mode)
{

	return 0;
}

/*only for qplay,config the device to network*/
void dlna_setNetwork(const char* ssid, const char* key)
{


}




void airplay_play(const char* uri)
{
    LOG_PRT();
}

void airplay_pause(void)
{
    LOG_PRT();
}

void airplay_stop(void)
{
    LOG_PRT();
    
    if( DLNA_CAN_NOT_PLAY() )
    {
        SuspendAirplay(fac.airplay_manager);
    }  
}

void airplay_next(void)
{
    LOG_PRT();
}

void airplay_previous(void)
{
    LOG_PRT();
}

void airplay_setNextUri(const char* uri)
{
    LOG_PRT();
}

void airplay_seek(int s_time)
{
   LOG_PRT();
}
int airplay_getDuration(void)
{
	return 0;
}
int airplay_getPosition(void)
{
	return 0;
}
void airplay_setMute(bool mute)
{

}
bool airplay_getMute(void)
{
	return false;
}

void airplay_setVolume (int vol)
{
    int new_vol;
    
	pthread_mutex_lock(&(ms_lk.mp_mutex_lock));

    if(AOInf_I2S == ffc_inst.player.get_ad())
    {
        new_vol = floor(vol*1.13); 
        if(0 != new_vol)
            new_vol += 142;
        
        if(OK != audio_snd_ctl(1, "hw:1,0", "Master Volume", &new_vol))
            LOG_PRT("error occur!");
    }

    else
    {
        new_vol = ceil(vol*0.62);
    
        if(OK != audio_snd_ctl(1, "default", "head phone volume", &new_vol))
            LOG_PRT("error occur!");
    }

	pthread_mutex_unlock(&(ms_lk.mp_mutex_lock));

}    


int airplay_getVolume (void)
{
	int old_vol, new_vol = 0;

	pthread_mutex_lock(&(ms_lk.mp_mutex_lock));
    
    if(AOInf_I2S == ffc_inst.player.get_ad())
    {
        if(OK == audio_snd_ctl(0, "hw:1,0", "Master Volume", &old_vol))
        {
            old_vol -= 142;
            if(old_vol < 0)
                old_vol = 0;
            new_vol = ceil(old_vol/1.13);
        }        
    }
    else
    {
        if(OK == audio_snd_ctl(0, "default", "head phone volume", &old_vol))
        {
            new_vol = floor(old_vol/0.62);
        }
    }

	pthread_mutex_unlock(&(ms_lk.mp_mutex_lock));

	return new_vol;
}  



void airplay_setPlayMode(int mode)
{
	
}


int airplay_getPlayMode(void)
{
	return 0;
}

int airplay_getState(void)
{
	return 0;
}

int airplay_rotate(int degree)
{
	return 0;
}

int airplay_scale(int ratio)
{
	return 0;
}

int airplay_setVideoSizeMode(int mode)
{
	return 0;
}
int airplay_setNetwork(const char* ssid, const char* key)
{
	return 0;
}

void airplay_initialize()
{
    player_dlna.stop();
}

void airplay_fetchFmt(int bit,int rate,int channel)
{
    LOG_PRT();
}


PlayerStruct player_dlna = {
	dlna_play,
	dlna_pause,
	dlna_stop,
	dlna_next,
	dlna_previous,
	dlna_setNextUri,
	dlna_seek,
	dlna_getDuration,
	dlna_getPosition,
	dlna_setMute,
	dlna_getMute,
	dlna_setVolume,
	dlna_getVolume,
	dlna_setPlaymode,
	dlna_getPlayMode,
	dlna_getState,
	dlna_rotate,
	dlna_scale,
	dlna_setVideoSizeMode,
	dlna_setNetwork
};

PlayerStruct player_airplay = {
	airplay_play,
	airplay_pause,
	airplay_stop,
	airplay_next,
	airplay_previous,
	airplay_setNextUri,
	airplay_seek,
	airplay_getDuration,
	airplay_getPosition,
	airplay_setMute,
	airplay_getMute,
	airplay_setVolume,
	airplay_getVolume,
	airplay_setPlayMode,
	airplay_getPlayMode,
	airplay_getState,
	airplay_rotate,
	airplay_scale,
	airplay_setVideoSizeMode,
	airplay_setNetwork,
	airplay_initialize,
	airplay_fetchFmt//suwenrong	
};


void close_service_func(void)
{
    
    LOG_PRT("stop server.....\n"); 

    if(!b_ServiceStarted)
        return;
    
    StopAirplay(fac.airplay_manager); 

    //sleep(1); 
    system("killall mdnsd"); 

    DestroyMediaRenderer(fac.dlna_manager, fac.dlna_renderer);	
    fac.dlna_renderer = NULL;

    //sleep(2); 
    DestroyDlnaManager(fac.dlna_manager); 
    fac.dlna_manager = NULL;
    
    DestroyAirplayManager(fac.airplay_manager);	
    fac.airplay_manager = NULL;

#ifdef __REVERSE_CONTROL__    
    extern void rc_exit(void);
    rc_exit();
#endif

#ifdef __MULTI_ROOM__
    multi_serv_exit();
#endif

    b_ServiceStarted = 0;
    
    LOG_PRT("close_service over!\n");
    
}


void open_service_func(void)
{
       
	//FILE *pstream;
   
    pthread_mutex_lock(&(ms_lk.mp_mutex_lock));
    
    if(b_ServiceStarted)
    {
        close_service_func();
    }

    LOG_PRT("start server.....\n"); 

    system("/home/mdnsd"); 
    sleep(2); 

	fac.airplay_manager = CreateAirplayManager();

    if(AOInf_I2S == ffc_inst.player.get_ad())
        SetAirplayAOInf(fac.airplay_manager, AOInf_I2S);
    else
        SetAirplayAOInf(fac.airplay_manager, AOInf_CODEC);

    StartAirplay(fac.airplay_manager, fac.device_name);
    
	SetAirplayPlayer(fac.airplay_manager, &player_airplay);

	fac.dlna_manager = CreateDlnaManager();

    LOG_PRT("device_name:%s, uuid:%s, device_id:%d\n", fac.device_name, fac.uuid, fac.device_id);

	fac.dlna_renderer = CreateMediaRenderer(fac.dlna_manager, fac.device_name, fac.uuid,
						fac.device_id, fac.manufacture_id, fac.pre_shared_key);
    
	SetDlnaPlayer(fac.dlna_renderer, &player_dlna);

#ifdef __REVERSE_CONTROL__
    printf("wait to connect android app for reverse control...\n");
    extern void* rc_init_pthread(void *arg);
    pthread_t tid2; 
    pthread_create(&tid2, NULL, &rc_init_pthread, NULL); 
#endif

    b_ServiceStarted = 1;
    
    pthread_mutex_unlock(&(ms_lk.mp_mutex_lock));

    LOG_PRT("service-start success!\n");
    
}


int reverse_event_handle(int cmd)
{
    switch(cmd)
    {
        case NEXT:
            
            if(IsQplay(fac.dlna_renderer))
                HWNext(fac.dlna_renderer);
            else if(IsAirplayPlaying(fac.airplay_manager))
                SetAirplayNext(fac.airplay_manager);
            else
                rc_commit(RC_NEXT);

            break;

        case PREVIOUS:
            
            if(IsQplay(fac.dlna_renderer))
                HWPrevious(fac.dlna_renderer);
            else if(IsAirplayPlaying(fac.airplay_manager))
                SetAirplayPrevious(fac.airplay_manager);
            else
                rc_commit(RC_PREVIOUS);

            break;

            
        case PAUSE:

            if(IsQplay(fac.dlna_renderer))
                HWPause(fac.dlna_renderer);
            //else if(IsAirplayPlaying(fac.airplay_manager))
            //    SetAirplayPlayPause(fac.airplay_manager);            
            else
                rc_commit(RC_PAUSE);                

            break;

        case PLAY:

            if(IsQplay(fac.dlna_renderer))
                HWPlay(fac.dlna_renderer);
            //else if(IsAirplayPlaying(fac.airplay_manager))
            //    SetAirplayPlayPause(fac.airplay_manager);            
            else
                rc_commit(RC_PLAY);                

            break;

        case VOLDOWN:
            
            if(IsAirplayPlaying(fac.airplay_manager))
                AdjAirplayVol(fac.airplay_manager, -1);
            else
                rc_commit(RC_VOICE);              

            break;

        case VOLUP:
            
            if(IsAirplayPlaying(fac.airplay_manager))
                AdjAirplayVol(fac.airplay_manager, 1);
            else
                rc_commit(RC_VOICE);              

            break;

        default:
            break;
    }



}
