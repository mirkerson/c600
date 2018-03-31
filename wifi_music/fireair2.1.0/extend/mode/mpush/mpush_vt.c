
#include "stdio.h"

#include "mpush.h"
#include "ad_type.h"


int mpush_init(void)
{
    int ret;

    
    ret = adplayer_init();
    if(OK != ret)
        return ret;    
}

void mpush_exit(void)
{
    adplayer_exit();
}


int mpush_play(char *uri)
{
    return ffc_inst.player.adplayer(AD_PLAY, uri);
}

void mpush_pause(void)
{
    ffc_inst.player.adplayer(AD_PAUSE, NULL);
}

void mpush_stop(void)
{
    ffc_inst.player.adplayer(AD_STOP, NULL);
}

void mpush_seek(int sec)
{
    ffc_inst.player.adplayer(AD_SEEK, &sec);
}

void mpush_get_file_attr(s_file_context *fc)
{
    ffc_inst.player.get_file_context(fc);
}

void mpush_set_file_attr(s_file_context *fc)
{
    ffc_inst.player.set_file_context(fc);
}

int mpush_get_status()
{
    return ffc_inst.player.adplayer(AD_GET_STATUS, NULL);
}

#ifdef __MULTI_ROOM__
void multi_serv_exit(void)
{
    ffc_inst.player.adplayer(MR_SERV_EXIT, NULL);
}
#endif

vtable pushmode = { 
        &mpush_init,
        &mpush_exit,
        &mpush_play, 
        NULL, 
        &mpush_pause, 
        &mpush_stop,
        &mpush_seek,
        &mpush_get_file_attr,
        &mpush_set_file_attr,
        &mpush_get_status
    };  

