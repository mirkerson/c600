
#include "example.h"
#include <stdlib.h>


static mexample *mexample_obj;

mexample* example_init(void)
{

    mexample_obj = (mexample *)malloc(sizeof(mexample));
    if(NULL == mexample_obj)
    {
        return NULL;
    }

    mexample_obj->vptr =  &examplemode;
    mexample_obj->check_next = example_check_next;
    mexample_obj->go_play_next = example_go_play_next;
    mexample_obj->go_play_prev = example_go_play_prev;

    mexample_obj->vptr->init();

    return mexample_obj;
    
}    

void example_exit(void)
{
    mexample_obj->vptr->unit();

    free(mexample_obj);
    mexample_obj = NULL;    
}


