
#include <stdio.h>
#include <unistd.h>

#include "api.h"
#include <sys/types.h>

extern pthread_mutex_t fa_mutex_lock;

void exit_cur_mode(int mode)
{
    
    switch(mode)
    {
        case MODE_PUSH:

            mi.mpobj->dlna->stop();
            mi.mpobj->airplay->stop();
            mi.mpobj->into_dormancy();

            break;

#ifdef __IFLY_VOICE__      
        case MODE_SPEECH:
      
            mi.miflyobj->vptr->stop();
            mi.miflyobj->release_res();

            break;
#endif

#ifdef __SIMPLE_PLAY__      
        case MODE_SIMPLE:
        
            mi.msimpleobj->vptr->stop();
            mi.msimpleobj->release_res();

            break;
#endif

        case MODE_LINEIN:

            mi.mblobj->vptr->stop();

            break;

#ifdef __SUPPORT_BT__
        case MODE_BT:

            mi.mblobj->vptr->stop();
            user_bt_shutdown();
            
            break;
#endif

#ifdef __ALI_MUSIC__
        case MODE_ALIMUSIC:

            mi.maliobj->vptr->stop();
            mi.maliobj->into_dormancy();
                    
            break;
#endif

        case MODE_LOCAL:

            mi.mlcobj->vptr->stop();
            mi.mlcobj->unplug();
            
            break;
            
        default:
            break;
            
    }
    
    //while(!play_over){usleep(10*1000);}

}




void switch_audio_mode(int pre_mode, int mode)
{
    int on=1,off=0;
    
	pthread_mutex_lock(&fa_mutex_lock);

    //printf("exit from mode: %d\n", pre_mode);
    
    //audio_set_mute(0);
    exit_cur_mode(pre_mode);
    //audio_set_mute(1);
    
    //printf("enter to mode: %d\n", mode);
    
    fa_obj.cur_mode = mode;
    
    
    switch(mode)
    {        
        case MODE_PUSH:
            printf("WIFI MODE------\n");
            
            ffc_inst.player.tone_play("/home/ad/mode1.mp3", 0);
            
            mi.mpobj->outof_dormancy();

            break;

#ifdef __ALI_MUSIC__            
        case MODE_ALIMUSIC:
            printf("ALI MODE------\n");
           
            ffc_inst.player.tone_play("/home/ad/mode2.mp3", 0);
           
            mi.maliobj->outof_dormancy();

            if( OK == ffc_inst.network.get_status() )
                mi.maliobj->sendmsg("play_done", NULL);
            
            break;
#endif

#ifdef __IFLY_VOICE__      
        case MODE_SPEECH:
            printf("SPEECH MODE------\n");
            
            ffc_inst.player.tone_play("/home/ad/mode2.mp3", 0); 
            
            break;
#endif

#ifdef __SIMPLE_PLAY__      
        case MODE_SIMPLE:
            printf("SIMPLE MODE------\n");

            ffc_inst.player.tone_play("/home/ad/mode6.mp3", 0);

            mi.msimpleobj->ch = 0;
            
            if( OK == mi.msimpleobj->url_parse(mi.msimpleobj->radio_src[mi.msimpleobj->ch]) )
                mi.msimpleobj->vptr->play(NULL);
            else
                ffc_inst.player.tone_play("/home/ad/searchfail.mp3", 0);
            
            break;
#endif


        case MODE_LINEIN:
            printf("LINE-IN MODE-------\n");

            ffc_inst.player.tone_play("/home/ad/mode3.mp3", 0);

            audio_set_mute(0);

            audio_snd_ctl(1, "default", "ADC mixer mute for linein" , &off);
        	audio_snd_ctl(1, "default", "ADC mixer mute for mic" , &off);
            audio_snd_ctl(1, "default", "ADC MIC Boost AMP en" , &off);
        	audio_snd_ctl(1, "default", "ADC mixer mute for FML" , &on);
            audio_snd_ctl(1, "default", "ADC mixer mute for FMR" , &on);            

            mi.mblobj->vptr->play(NULL);
            
            sleep(1);
            
            audio_set_mute(1);

            break;

#ifdef __SUPPORT_BT__
        case MODE_BT:
            printf("BT MODE-------\n");

            ffc_inst.player.tone_play("/home/ad/mode5.mp3", 0);

            user_bt_init();
            
            audio_set_mute(0);

           	audio_snd_ctl(1, "default", "ADC mixer mute for FML" , &off);
            audio_snd_ctl(1, "default", "ADC mixer mute for FMR" , &off);            
        	audio_snd_ctl(1, "default", "ADC mixer mute for mic" , &off);
            audio_snd_ctl(1, "default", "ADC MIC Boost AMP en" , &off);
            audio_snd_ctl(1, "default", "ADC mixer mute for linein" , &on);            

            mi.mblobj->vptr->play(NULL);
            
            sleep(1);
            
            audio_set_mute(1);

            break;
#endif

        case MODE_LOCAL:
            printf("LOCAL MODE-------\n");

            ffc_inst.player.tone_play("/home/ad/mode4.mp3", 0);
            
            mi.mlcobj->insert();

            mi.mlcobj->vptr->play(NULL);
            break;
            
         default:
            break;

    }

	pthread_mutex_unlock(&fa_mutex_lock);

}



