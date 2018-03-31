
#ifndef _PLAYER_API_H_
#define _PLAYER_API_H_

#include "common_def.h"

extern int adplayer(int cmd, void *arg);
extern void adplayer_get_file_context(s_file_context *fc);
extern void adplayer_set_file_context(s_file_context *fc);
extern void tone_start(void* arg, int type);
extern void switchAdout(void);
extern int getAdout(void);

extern void player_set_external_callback(void *ffc);

#endif

