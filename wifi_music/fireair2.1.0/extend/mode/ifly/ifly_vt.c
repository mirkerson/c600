
#ifdef __IFLY_VOICE__

#include <unistd.h>

#include "ad_type.h"
#include "ifly.h"

extern P_LIST_URL url_list;
static s_file_context ifly_sfc;

int mifly_init(void)
{
    int ret;

    memset(ifly_sfc.url, 0, sizeof(ifly_sfc.url));

    ret = adplayer_init();
    if(OK != ret)
        return ret;    
}    


void mifly_exit(void)
{
    adplayer_exit();    
}


int mifly_play(char *uri)
{
    int ret;
    char cmd[128]={0};

    if(IS_PAUSE == ffc_inst.player.adplayer(AD_GET_STATUS, NULL))
    {
        ret = ffc_inst.player.adplayer(AD_PLAY, NULL);

        return ret;
    }
    
    ret = demo_voice_kdxf(REC_FILE);

    if(VOICE_MUSIC == ret)
    {
        sprintf(cmd,"rm \"%s\"", REC_FILE);    
        system(cmd);    
        
        ret = ffc_inst.player.adplayer(AD_PLAY, url_list->url);
        return ret;
    }
    else if(VOICE_WEATHER == ret)
    {
        ret = ffc_inst.player.adplayer(AD_PLAY, TTS_FILE);
        return ret;
    }
 
}

void mifly_pause(void)
{
    ffc_inst.player.adplayer(AD_PAUSE, NULL);
}

void mifly_stop(void)
{    
    ffc_inst.player.adplayer(AD_STOP_RECORD, NULL);
    
    ffc_inst.player.adplayer(AD_STOP, NULL);
    //delete_url_list();

    usleep(100*1000);
}

void mifly_record(void)
{
    int on=1, off=0;

   	audio_snd_ctl(1, "default", "ADC mixer mute for FML" , &off);
    audio_snd_ctl(1, "default", "ADC mixer mute for FMR" , &off);
	audio_snd_ctl(1, "default", "ADC mixer mute for mic" , &on);
    audio_snd_ctl(1, "default", "ADC MIC Boost AMP en" , &on);

    ifly_sfc.channels = 1;
    ifly_sfc.samplerate = 16000;
    strcpy(ifly_sfc.url, REC_FILE);

    ffc_inst.player.adplayer(AD_RECORD, &ifly_sfc);

}

void mifly_get_file_attr(s_file_context *fc)
{
    ffc_inst.player.get_file_context(fc);
}

void mifly_set_file_attr(s_file_context *fc)
{
    ffc_inst.player.set_file_context(fc);
}

int mifly_get_status()
{
    return ffc_inst.player.adplayer(AD_GET_STATUS, NULL);
}


vtable iflymode = { 
        &mifly_init,
        &mifly_exit,
        &mifly_play, 
        &mifly_record, 
        &mifly_pause, 
        &mifly_stop,
        NULL,
        &mifly_get_file_attr,
        &mifly_set_file_attr,
        &mifly_get_status
    }; 


int ifly_check_next(void)
{    
    return ffc_inst.player.adplayer(AD_READY_GO_NEXT, NULL);
}

void ifly_go_play_next(void)
{
    P_LIST_URL url_nod = NULL;
    
    if(NULL == url_list)
        return;

    url_nod = url_list;
    
    do{
        url_list = url_list->next;

        if(url_nod == url_list)
            return;
    }
    while(NULL == url_list->url);

    ffc_inst.player.adplayer(AD_STOP, NULL);
    ffc_inst.player.adplayer(AD_PLAY, url_list->url);
    
}    


void ifly_go_play_prev(void)
{
    P_LIST_URL url_nod = NULL;
    
    if(NULL == url_list)
        return;

    url_nod = url_list;
    
    do{
        url_list = url_list->prev;

        if(url_nod == url_list)
            return;
    }
    while(NULL == url_list->url);

    ffc_inst.player.adplayer(AD_STOP, NULL);
    ffc_inst.player.adplayer(AD_PLAY, url_list->url);
    
}    



#endif

