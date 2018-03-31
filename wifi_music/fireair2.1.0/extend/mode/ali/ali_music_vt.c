
#ifdef __ALI_MUSIC__


#include <unistd.h>
#include <pthread.h>

#include "ad_type.h"
#include "ali_music.h"

extern P_LIST_URL smp_list;
extern pnode offlist_save;
static int ali_to_next = 0;
//extern int ali_record_en;
static s_file_context ali_sfc;

int mali_init(void)
{
    int ret;
    
    memset(ali_sfc.url, 0, sizeof(ali_sfc.url));

    ret = adplayer_init();
    if(OK != ret)
        return ret;    
}    


void mali_exit(void)
{
    adplayer_exit();    
}


int mali_play(char *uri)
{
    int ret;

    if(IS_PAUSE == ffc_inst.player.adplayer(AD_GET_STATUS, NULL))
    {
        ret = ffc_inst.player.adplayer(AD_PLAY, NULL);

        return ret;
    }

    if(OFFLINE_MODE == get_alistate())
    {
        if(NULL == offlist_save)
            return ERROR;
        
        ret = ffc_inst.player.adplayer(AD_PLAY, offlist_save->fullname);

        return ret;
    }
    else
    {
        if(NULL == uri)
            return ERROR;
        
        if( (NULL == strstr(uri, ".m3u8")) && (NULL == strstr(uri, "/home/voice/")) )            
            download_file(uri);
        
        ret = ffc_inst.player.adplayer(AD_PLAY, uri);
        if(OK != ret)
        {
            return ret;
        }        
        else
        {        
            ali_to_next = 0;    
            return ret;
        }
    }

        
}

void mali_pause(void)
{
    ffc_inst.player.adplayer(AD_PAUSE, NULL);
}

void mali_stop(void)
{
    ffc_inst.player.adplayer(AD_STOP_RECORD, NULL);

    ffc_inst.player.adplayer(AD_STOP, NULL);

    usleep(100*1000);
}

void mali_record(void)
{
    int on=1, off=0;

   	audio_snd_ctl(1, "default", "ADC mixer mute for FML" , &off);
    audio_snd_ctl(1, "default", "ADC mixer mute for FMR" , &off);
	audio_snd_ctl(1, "default", "ADC mixer mute for mic" , &on);
    audio_snd_ctl(1, "default", "ADC MIC Boost AMP en" , &on);

    ali_sfc.channels = 1;
    ali_sfc.samplerate = 16000;
    strcpy(ali_sfc.url, REC_ALI_FILE);

    ffc_inst.player.adplayer(AD_RECORD, &ali_sfc);
}

void mali_get_file_attr(s_file_context *fc)
{
    ffc_inst.player.get_file_context(fc);
}

void mali_set_file_attr(s_file_context *fc)
{
    ffc_inst.player.set_file_context(fc);
}

int mali_get_status()
{
    return ffc_inst.player.adplayer(AD_GET_STATUS, NULL);
}

int ali_analyze(void)
{      
    return analyze_ali_voice();
}

int ali_check_next(void)
{      
    return ffc_inst.player.adplayer(AD_READY_GO_NEXT, NULL);
}

void ali_go_play_next(void)
{
    LOG_PRT();
    
    if(OFFLINE_MODE == get_alistate())
    {
        if(NULL == offlist_save)
            return;
        
        offlist_save = offlist_save->next;
        ffc_inst.player.adplayer(AD_STOP, NULL);
        ffc_inst.player.adplayer(AD_PLAY, offlist_save->fullname);
    }
    else
    {
        if(ali_to_next)
            return;    

        LOG_PRT();
        
        ali_notification_event("play_done", NULL);
        ali_to_next = 1;  
    }

  

} 


vtable alimode = { 
        &mali_init,
        &mali_exit,
        &mali_play, 
        &mali_record, 
        &mali_pause, 
        &mali_stop,
        NULL,
        &mali_get_file_attr,
        &mali_set_file_attr,
        &mali_get_status
    }; 

#endif

