
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <alsa/asoundlib.h>
#include <alsa/pcm.h>

#include "common_def.h"
#include "bl.h"
#include "ad_type.h"
#include "factory_api.h"


static s_file_context* bl_fc;
static int pause_sta = 0;

int mbl_play(char *url)
{
    return ffc_inst.player.adplayer(AD_AUX_START, NULL);
}

void mbl_stop(void)
{
    pause_sta = 0;

    ffc_inst.player.adplayer(AD_AUX_STOP, NULL);

}

void mbl_pause(void)
{
    pause_sta = 1;
}

int mbl_init(void)
{
    bl_fc = (s_file_context *)malloc(sizeof(s_file_context));
    if(NULL == bl_fc)
        return ERROR;
}

void mbl_exit(void)
{

    //while(NULL != linein_head){usleep(10000);}
    
    free(bl_fc);
    bl_fc = NULL;
}

void mbl_get_file_attr(s_file_context *fc)
{
    ffc_inst.player.get_file_context(fc);
}

void mbl_set_file_attr(s_file_context *fc)
{
    ffc_inst.player.set_file_context(fc);
}



vtable blmode = { 
        &mbl_init,
        &mbl_exit,
        &mbl_play, 
        NULL, 
        &mbl_pause, 
        &mbl_stop,
        NULL,
        &mbl_get_file_attr,
        &mbl_set_file_attr,
        NULL
    };  

int bt_is_pause(void)
{
    return pause_sta;
}

int bt_clear_sta(void)
{
    pause_sta = 0;
}

