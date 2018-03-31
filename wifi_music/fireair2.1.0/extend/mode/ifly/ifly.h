
#ifndef __IFLY_H
#define __IFLY_H


#include "ad_type.h"

typedef struct  
{  
    vtable *vptr;  
    int (*check_next)(void);
    void (*go_play_next)(void);
    void (*go_play_prev)(void);
    void (*release_res)(void);
}mifly;
extern mifly *mifly_obj;


extern vtable iflymode;

#define REC_FILE "/tmp/rec_allwinner.wav"
#define TTS_FILE "/tmp/text_to_speech_test.wav"

#define VOICE_MUSIC     0x1
#define VOICE_WEATHER   0x2
#define VOICE_UNKNOW    0xff


int ifly_check_next(void);
void ifly_go_play_next(void);
void ifly_go_play_prev(void);

#endif

