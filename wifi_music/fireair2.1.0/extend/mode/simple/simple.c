
#ifdef __SIMPLE_PLAY__


#include <stdlib.h>
#include "simple.h"

extern void delete_smp_list(void);

msimple *msimple_obj; 

#define XI_MAX  7 

//ximalaya http request
char *ximalaya[XI_MAX] = 
{
    "http://3rd.ximalaya.com/explore/tracks?i_am=test20150518&category_id=2&tag=排行榜&condition=daily&page=1&per_page=20&uni=xxx",                  //music
    "http://3rd.ximalaya.com/explore/tracks?i_am=test20150518&category_id=5&tag=%E5%A4%96%E8%AF%AD&condition=daily&page=1&per_page=20&uni=xxx",         //english
    "http://3rd.ximalaya.com/explore/tracks?i_am=test20150518&category_id=3&tag=%E7%8E%84%E5%B9%BB&condition=daily&page=1&per_page=20&uni=xxx",         //book
    "http://3rd.ximalaya.com/explore/tracks?i_am=test20150518&category_id=14&tag=%E5%8E%86%E5%8F%B2&condition=daily&page=1&per_page=20&uni=xxx",        //Lecture Room
    "http://3rd.ximalaya.com/explore/tracks?i_am=test20150518&category_id=17&tag=%E9%9F%B3%E4%B9%90&condition=daily&page=1&per_page=20&uni=xxx",        //FM
    "http://3rd.ximalaya.com/explore/tracks?i_am=test20150518&category_id=12&tag=%E5%BE%B7%E4%BA%91%E7%A4%BE&condition=daily&page=1&per_page=20&uni=xxx",//comic
    "http://3rd.ximalaya.com/explore/tracks?i_am=test20150518&category_id=6&tag=%E5%84%BF%E7%AB%A5%E6%95%85%E4%BA%8B&condition=daily&page=1&per_page=20&uni=xxx"//children
};


extern int curl_parse(char *url);

msimple* simple_init(void)
{

    msimple_obj = (msimple *)malloc(sizeof(msimple));
    if(NULL == msimple_obj)
    {
        return NULL;
    }

    msimple_obj->vptr =  &simplemode;
    msimple_obj->check_next = simple_check_next;
    msimple_obj->go_play_next = simple_go_play_next;
    msimple_obj->go_play_prev = simple_go_play_prev;
    msimple_obj->url_parse = curl_parse;   	 //to parse url
    msimple_obj->release_res = delete_smp_list;

    msimple_obj->ch = 0;
    msimple_obj->radio_src = ximalaya; 
    
    msimple_obj->vptr->init();

    return msimple_obj;
    
}    

void simple_exit(void)
{
    msimple_obj->vptr->unit();

    free(msimple_obj);
    msimple_obj = NULL;    
}

#endif

