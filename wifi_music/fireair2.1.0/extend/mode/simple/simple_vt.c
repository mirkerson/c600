
#ifdef __SIMPLE_PLAY__

#include <unistd.h>

#include "ad_type.h"
#include "simple.h"

extern P_LIST_URL smp_list;


int msimple_init(void)
{
    int ret;

    ret = adplayer_init();
    if(OK != ret)
        return ret;    
}    


void msimple_exit(void)
{
    adplayer_exit();    
}


int msimple_play(char *uri)
{
    return ffc_inst.player.adplayer(AD_PLAY, smp_list->url);
}

void msimple_pause(void)
{
    ffc_inst.player.adplayer(AD_PAUSE, NULL);
}

void msimple_stop(void)
{    
    ffc_inst.player.adplayer(AD_STOP, NULL);
    //delete_smp_list();
}



void msimple_get_file_attr(s_file_context *fc)
{
    ffc_inst.player.get_file_context(fc);
}

void msimple_set_file_attr(s_file_context *fc)
{
    ffc_inst.player.set_file_context(fc);
}

int msimple_get_status()
{
    return ffc_inst.player.adplayer(AD_GET_STATUS, NULL);
}


vtable simplemode = { 
        &msimple_init,
        &msimple_exit,
        &msimple_play, 
        NULL, 
        &msimple_pause, 
        &msimple_stop,
        NULL,
        &msimple_get_file_attr,
        &msimple_set_file_attr,
        &msimple_get_status
    }; 


int simple_check_next(void)
{    
    return ffc_inst.player.adplayer(AD_READY_GO_NEXT, NULL);
}

void simple_go_play_next(void)
{
    P_LIST_URL url_nod = NULL;
    
    if(NULL == smp_list)
        return;

    url_nod = smp_list;
    
    do{
        smp_list = smp_list->next;

        if(url_nod == smp_list)
            return;
    }
    while(NULL == smp_list->url);

    ffc_inst.player.adplayer(AD_STOP, NULL);
    ffc_inst.player.adplayer(AD_PLAY, smp_list->url);
    
}    


void simple_go_play_prev(void)
{
    P_LIST_URL url_nod = NULL;
    
    if(NULL == smp_list)
        return;

    url_nod = smp_list;
    
    do{
        smp_list = smp_list->prev;

        if(url_nod == smp_list)
            return;
    }
    while(NULL == smp_list->url);

    ffc_inst.player.adplayer(AD_STOP, NULL);
    ffc_inst.player.adplayer(AD_PLAY, smp_list->url);
    
}    



#endif

