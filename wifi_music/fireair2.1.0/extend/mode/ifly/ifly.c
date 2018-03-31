
#ifdef __IFLY_VOICE__


#include <stdlib.h>
#include "ifly.h"

extern void delete_url_list(void);

mifly *mifly_obj;


mifly* ifly_init(void)
{

    mifly_obj = (mifly *)malloc(sizeof(mifly));
    if(NULL == mifly_obj)
    {
        return NULL;
    }

    mifly_obj->vptr =  &iflymode;
    mifly_obj->check_next = ifly_check_next;
    mifly_obj->go_play_next = ifly_go_play_next;
    mifly_obj->go_play_prev = ifly_go_play_prev;

    mifly_obj->release_res = delete_url_list;

    mifly_obj->vptr->init();



    return mifly_obj;
    
}    

void ifly_exit(void)
{

    mifly_obj->vptr->unit();

    free(mifly_obj);
    mifly_obj = NULL;    
}

#endif

