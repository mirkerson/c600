
#include <signal.h>

#include "local.h"
#include <stdlib.h>


static mlocal *mlc_obj;


mlocal* local_init(void)
{

    mlc_obj = (mlocal *)malloc(sizeof(mlocal));
    if(NULL == mlc_obj)
    {
        return NULL;
    }

    mlc_obj->vptr =  &localmode;
    mlc_obj->insert = sd_insert;
    mlc_obj->unplug = sd_unplug;
    mlc_obj->check_next = local_check_next;
    mlc_obj->go_play_next = local_go_play_next;
    mlc_obj->go_play_prev = local_go_play_prev;

    mlc_obj->vptr->init();

    system("echo 1 > /sys/class/gpio/gpio0/value");//enable usb power

    return mlc_obj;
    
}    

void local_exit(void)
{

    mlc_obj->vptr->unit();

    free(mlc_obj);
    mlc_obj = NULL;    
}


