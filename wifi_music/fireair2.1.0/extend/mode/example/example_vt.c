
#include "ad_type.h"
#include "example.h"



int mexample_init(void)
{
    int ret;
    
    ret = adplayer_init();
    if(OK != ret)
        return ret;    
}

void mexample_exit(void)
{
    adplayer_exit();
}

int mexample_play(char *uri)
{	
    return ffc_inst.player.adplayer(AD_PLAY, uri);
}

void mexample_pause(void)
{
    ffc_inst.player.adplayer(AD_PAUSE, NULL);
}

void mexample_stop(void)
{
    ffc_inst.player.adplayer(AD_STOP, NULL);
}

void mexample_record(void)
{
	LOG_PRT();
}

void mexample_seek(int sec)
{
	ffc_inst.player.adplayer(AD_SEEK, &sec);
}

void mexample_get_file_attr(s_file_context *fc)
{
    ffc_inst.player.get_file_context(fc);
}

void mexample_set_file_attr(s_file_context *fc)
{
    ffc_inst.player.set_file_context(fc);
}

int mexample_get_status()
{
    return ffc_inst.player.adplayer(AD_GET_STATUS, NULL);
}


vtable examplemode = { 
        &mexample_init,
        &mexample_exit,
        &mexample_play, 
        &mexample_record, 
        &mexample_pause, 
        &mexample_stop,
        &mexample_seek,
        &mexample_get_file_attr,
        &mexample_set_file_attr,
        &mexample_get_status
    }; 

int example_check_next(void)
{    
    return ffc_inst.player.adplayer(AD_READY_GO_NEXT, NULL);
}

void example_go_play_next(void)
{
    LOG_PRT();
}

void example_go_play_prev(void)
{
    LOG_PRT();
}

