
#ifndef __SIMPLE_H
#define __SIMPLE_H

#include "factory_api.h"

typedef struct  
{  
    vtable *vptr;  
    int (*check_next)(void);
    void (*go_play_next)(void);
    void (*go_play_prev)(void);
    int (*url_parse)(char *url);
    void (*release_res)(void);    
    char **radio_src;
    int   ch;

}msimple;

extern vtable simplemode;

int simple_check_next(void);
void simple_go_play_next(void);
void simple_go_play_prev(void);

#endif

