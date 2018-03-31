

#include "bl.h"
#include <stdlib.h>


static mbl *mbl_obj;


extern int bt_is_pause(void);
extern int bt_clear_sta(void);

mbl* bl_init(void)
{
    
    mbl_obj = (mbl *)malloc(sizeof(mbl));
    if(NULL == mbl_obj)
    {
        return NULL;
    }
        
    mbl_obj->is_pause = bt_is_pause;
    
    mbl_obj->clear_sta = bt_clear_sta;

    mbl_obj->vptr =  &blmode;

    mbl_obj->vptr->init();

    return mbl_obj;
    
}    

void bl_exit(void)
{

    mbl_obj->vptr->unit();

    free(mbl_obj);
    mbl_obj = NULL;    
}


