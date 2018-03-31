
#include "common_def.h"
#include "ad_type.h"


#define BT     0x55
#define AUX    0x66


typedef struct  
{  
    vtable *vptr;  
    int (*is_pause)(void);   
    int (*clear_sta)(void);  

}mbl;
extern vtable blmode;


