
#ifndef _COMMON_DEF_H_
#define _COMMON_DEF_H_

#include <stdio.h>
#include <stdlib.h>


#define ERROR  1
#define OK     0

#define __LOG_PRT__  
#ifdef __LOG_PRT__  
#define LOG_PRT(format,...) printf("%s line: %d===="format"\n", __func__, __LINE__, ##__VA_ARGS__)  
#else  
#define LOG_PRT(format,...)  
#endif


enum {
    CLASS_PUSH = 0,
#ifdef __IFLY_VOICE__        
    CLASS_SPEECH,    
#endif
#ifdef __SIMPLE_PLAY__        
    CLASS_SIMPLE,    
#endif
#ifdef __ALI_MUSIC__        
    CLASS_ALIMUSIC,    
#endif
#ifdef __SUPPORT_BT__ 
    CLASS_LINEIN_BT,
#endif    
    CLASS_LOCAL,

    
    CLASS_MAX
};

enum {
    MODE_PUSH = 0,
#ifdef __IFLY_VOICE__
    MODE_SPEECH,
#endif    
#ifdef __SIMPLE_PLAY__        
    MODE_SIMPLE,    
#endif

    MODE_LINEIN,
#ifdef __SUPPORT_BT__    
    MODE_BT,
#endif    
    MODE_LOCAL,

#ifdef __ALI_MUSIC__        
    MODE_ALIMUSIC,    
#endif


    
    MODE_MAX
};


enum {
    KEY_UNVALID = 0,
    KEY_S_MODE,
    KEY_S_PLAYPAUSE,
    KEY_S_VOLDOWN,
    KEY_S_VOLUP,    
    KEY_S_SACONFIG,
    KEY_S_VOICE_START,
    KEY_S_VOICE_END,
    KEY_S_AP,
    KEY_S_AP_STA,
    KEY_S_NEXT,
    KEY_S_PREVIOUS,    
    KEY_S_SETAD,
    KEY_S_END
};


typedef struct file_context {
    int sample_bits;
    int samplerate;    
    int channels;
    int duaration;//s
    int postion;//s
#ifdef __MULTI_ROOM__
    int pos_num;
    int pk_num;
#endif    
    int format;
    char url[1024];    
    unsigned int buff_len;
    int seek_time;//s
    
    int content_type;
    char *content;
    int content_size;
    int speedx;
        
} s_file_context;

typedef struct  
{  
    int  (*init)(void);
    void (*unit)(void);
    int  (*play)(char *uri);  
    void (*record)(void);
    void (*pause)(void);   
    void (*stop)(void);      
    void (*seek)(int);  
    void (*get_file_attr)(s_file_context *fc);
    void (*set_file_attr)(s_file_context *fc);
    int (*get_status)(void);
}vtable; 

typedef struct list_url{
    char *url;    
    struct list_url *next;
    struct list_url *prev;
}S_LIST_URL,*P_LIST_URL;

typedef struct  
{  
    char firm_version[32];
    char url[128];	

}update_info;

typedef struct meta_data {
    char url[1024];
    char title[64];
    char artist[64];
} S_META_DATA, *P_META_DATA;

typedef struct st_data {

    double rate_percent;
    double tempo_percent;
    double rate;
    double tempo;
    double pitch;

} S_ST_DATA, *P_ST_DATA;

#endif

