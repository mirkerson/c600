
#include "factory_api.h"

typedef struct  
{  
    vtable *vptr;  
    int (*check_next)(void);
    void (*go_play_next)(void);
    void (*go_play_prev)(void);

}mexample;
extern vtable examplemode;

int example_check_next(void);
void example_go_play_next(void);
void example_go_play_prev(void);
