

#include <stdio.h>
#include <stdbool.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <linux/input.h>
#include <math.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <pthread.h>

#include <alsa/asoundlib.h>
#include <alsa/pcm.h>

#include "api.h"
#include "multiroom.h"
#include "swift_link.h"
#include "ad_type.h"

#define POWER_OFF_TIME			4

volatile int b_powerKey_Down = 0;	
volatile struct timeval powerKey_Down_t;

static int b_ExtConfigSta; //contrl wifi connect; 0:dont connect    1:start connect

pthread_mutex_t fa_mutex_lock;
static pthread_mutex_t alsa_mutex_lock;

//mpush *f_mp_obj = NULL;
static int wifi_interrpt_flag = 0;
static int ap_sta_flag = 0;

factory_func_callback ffc_inst;

void fa_new(MODE_INST* m, int mode);
void fa_del(MODE_INST* m, int mode);


MODE_INST mi;
FIREAIR fa_obj = { fa_new, fa_del, MODE_MAX }; 


void set_sa_config_interrupt(int sta)
{
    wifi_interrpt_flag = sta;
}

int get_sa_config_interrupt(void)
{
    return wifi_interrpt_flag;
}

void fa_new(MODE_INST* m, int mode)
{

    switch(mode)
    {
        case CLASS_PUSH:
            m->mpobj = push_init();
            break;
            
        case CLASS_LINEIN_BT:
            m->mblobj = bl_init();
            break;
            
        case CLASS_LOCAL:
            m->mlcobj = local_init();
            break;

#ifdef __IFLY_VOICE__
        case CLASS_SPEECH:
            m->miflyobj = ifly_init();
            
            break;
#endif

#ifdef __SIMPLE_PLAY__
        case CLASS_SIMPLE:
            m->msimpleobj = simple_init();
             
            break;
#endif

#ifdef __ALI_MUSIC__
        case CLASS_ALIMUSIC:
            m->maliobj = ali_init();
             
            break;
#endif
            
        default:
            break;
            
    }
}

void fa_del(MODE_INST* m, int mode)
{
    switch(mode)
    {
        case CLASS_PUSH:
            push_exit();
            
            break;
        case CLASS_LINEIN_BT:
            bl_exit();
            
            break;

        case CLASS_LOCAL:
            local_exit();
            
            break;

#ifdef __IFLY_VOICE__
        case CLASS_SPEECH:
            ifly_exit();
           
            break;
#endif 

#ifdef __SIMPLE_PLAY__
        case CLASS_SIMPLE:
            simple_exit();
            
            break;
#endif

#ifdef __ALI_MUSIC__
        case CLASS_ALIMUSIC:
            ali_exit();
             
            break;
#endif

        default:
            break;
            
    }
}


void auto_play(int netsta)
{
    int on = 1;
    int off = 0;
    
    if(netsta > 0)
    {
    
        #ifdef __IFLY_VOICE__                        
        if(MODE_SPEECH == fa_obj.cur_mode)
        {
            //mi.miflyobj->vptr->play(NULL);
        }
        #endif
    
        #ifdef __SIMPLE_PLAY__
        if(MODE_SIMPLE == fa_obj.cur_mode)
        {
            mi.msimpleobj->vptr->play(NULL);
        }
        #endif
    }
    
    if(MODE_LOCAL == fa_obj.cur_mode)
    {
        mi.mlcobj->vptr->play(NULL);
    }
  
#ifdef __SUPPORT_BT__
    else if(MODE_BT == fa_obj.cur_mode)
    {
        audio_set_mute(0);

        audio_snd_ctl(1, "default", "ADC mixer mute for FML" , &off);
        audio_snd_ctl(1, "default", "ADC mixer mute for FMR" , &off);            
        audio_snd_ctl(1, "default", "ADC mixer mute for mic" , &off);
        audio_snd_ctl(1, "default", "ADC MIC Boost AMP en" , &off);
        audio_snd_ctl(1, "default", "ADC mixer mute for linein" , &on);            

        mi.mblobj->vptr->play(NULL);
        
        sleep(1);
        
        audio_set_mute(1);         
    }
#endif  
    else if(MODE_LINEIN == fa_obj.cur_mode)
    {
        audio_set_mute(0);
        
        audio_snd_ctl(1, "default", "ADC mixer mute for linein" , &off);
        audio_snd_ctl(1, "default", "ADC mixer mute for mic" , &off);
        audio_snd_ctl(1, "default", "ADC MIC Boost AMP en" , &off);
        audio_snd_ctl(1, "default", "ADC mixer mute for FML" , &on);
        audio_snd_ctl(1, "default", "ADC mixer mute for FMR" , &on);            
        
        mi.mblobj->vptr->play(NULL);
        
        sleep(1);
        
        audio_set_mute(1);

    }
#if 0//def __ALI_MUSIC__
    else if(MODE_ALIMUSIC == fa_obj.cur_mode)
    {
        s_file_context fc;
        mi.maliobj->vptr->get_file_attr(&fc);        
        mi.maliobj->vptr->play(fc.url); 
    }
#endif 

}

void auto_stop(void)
{
    if(MODE_PUSH == fa_obj.cur_mode)
    {
        mi.mpobj->vptr->stop();
    }
    else if(MODE_LOCAL == fa_obj.cur_mode)
    {
        mi.mlcobj->vptr->stop();
    }
#ifdef __IFLY_VOICE__                        
    else if(MODE_SPEECH == fa_obj.cur_mode)
    {
        mi.miflyobj->vptr->stop();
    }
#endif
#ifdef __SIMPLE_PLAY__
    else if(MODE_SIMPLE == fa_obj.cur_mode)
    {
        mi.msimpleobj->vptr->stop();
    }
#endif  
#ifdef __SUPPORT_BT__
    else if(MODE_BT == fa_obj.cur_mode)
    {
        mi.mblobj->vptr->stop();
    }
#endif  
#ifdef __ALI_MUSIC__
    else if(MODE_ALIMUSIC == fa_obj.cur_mode)
    {
        mi.maliobj->vptr->stop();
    }
#endif  

    else if(MODE_LINEIN == fa_obj.cur_mode)
    {
        mi.mblobj->vptr->stop();
    }

}

void auto_play_pause(void)
{
    LOG_PRT("cur_mode=%d", fa_obj.cur_mode);
    
    if(MODE_PUSH == fa_obj.cur_mode)
    {
        if(IS_PLAY == mi.mpobj->vptr->get_status())
            mi.mpobj->commit(PAUSE);
        else if(IS_PAUSE == mi.mpobj->vptr->get_status())
            mi.mpobj->commit(PLAY);
    }
    else if(MODE_LOCAL == fa_obj.cur_mode)
    {
        if(IS_PLAY == mi.mlcobj->vptr->get_status())
            mi.mlcobj->vptr->pause();
        else if(IS_PAUSE == mi.mlcobj->vptr->get_status())
            mi.mlcobj->vptr->play(NULL);
    }
#ifdef __ALI_MUSIC__                        
    else if(MODE_ALIMUSIC == fa_obj.cur_mode)
    {
        if(IS_PLAY == mi.maliobj->vptr->get_status())
            mi.maliobj->vptr->pause();
        else if(IS_PAUSE == mi.maliobj->vptr->get_status())
            mi.maliobj->vptr->play(NULL);
    }
#endif    
#ifdef __IFLY_VOICE__                        
    else if(MODE_SPEECH == fa_obj.cur_mode)
    {
        if(IS_PLAY == mi.miflyobj->vptr->get_status())
            mi.miflyobj->vptr->pause();
        else if(IS_PAUSE == mi.miflyobj->vptr->get_status())
            mi.miflyobj->vptr->play(NULL);
    }
#endif
#ifdef __SIMPLE_PLAY__
    else if(MODE_SIMPLE == fa_obj.cur_mode)
    {
        if(IS_PLAY == mi.msimpleobj->vptr->get_status())
            mi.msimpleobj->vptr->pause();
        else if(IS_PAUSE == mi.msimpleobj->vptr->get_status())
            mi.msimpleobj->vptr->play(NULL);        
    }
#endif  
#ifdef __SUPPORT_BT__
    else if(MODE_BT == fa_obj.cur_mode)
    {
        if(0 == mi.mblobj->is_pause())
        {
            mi.mblobj->vptr->pause();
            user_bt_play_pause();
        }
        else
        {
            user_bt_play_pause();
            mi.mblobj->clear_sta();
        }
    }
#endif  


}


void *press_power_off_func(void *arg)
{    
    pthread_detach(pthread_self());

    struct timeval now_t;
    
    while(b_powerKey_Down)
    {
    	gettimeofday(&now_t,NULL);
        
    	if( ((now_t.tv_sec - powerKey_Down_t.tv_sec)*1000000 + (now_t.tv_usec - powerKey_Down_t.tv_usec))
            > (POWER_OFF_TIME * 1000000) )
    	{    		
    		//long press powerKey 4s,then poweroff
    		b_powerKey_Down = 0;
            //adc_switch = 1;
            sleep(1);

            ffc_inst.player.tone_play("/home/ad/guanji.mp3", 0);

            system("echo 0   >  /sys/class/gpio/gpio0/value");
    		system("poweroff");
    	}

        usleep(100*1000);	//100 ms
    }

    pthread_exit(NULL);
    
}

void press_power_off(void)
{
	int ret;
	pthread_t tid;
	ret = pthread_create(&tid, NULL, &press_power_off_func, NULL);  
	if(ret != 0)  	 
		perror("pthread_create failed\n");  
}

static bool wifi_configing = false;
static pthread_t wifi_config_pthread;
static pthread_mutex_t wifi_config_lock = PTHREAD_MUTEX_INITIALIZER;
void *wifi_config_pthread_func(void *arg)
{
    int ret;
    wifi_configing = true;

    auto_stop();
    
    ffc_inst.driver.set_status(LED_WIFI_CONNECTED);                        
    
    mi.mpobj->close_service();                        
      
#ifdef __ALI_MUSIC__
    mi.maliobj->close_service();
    ffc_inst.player.tone_play("/home/ad/yijianpeizhi.mp3", 0);
    ret = mi.maliobj->simple_config();
#else
    ffc_inst.player.tone_play("/home/ad/shengwenlianwang.mp3", 0);
    ret = swift_link_connect_ap();
#endif
    LOG_PRT("ret=%d", ret);
    if(OK != ret)
    {
        if(ERROR == ret)
            ffc_inst.player.tone_play("/home/ad/lianwangshibai.mp3", 0);//tone_start("/home/ad/jianchawangluo.mp3");
        
        ffc_inst.driver.set_status(LED_WIFI_DISCONNECT);   
       
        auto_play(0);
    }                            
    else
    {
        system("/home/tinyplay /home/ad/lianwangok.wav");
        system("/home/tinyplay /home/ad/lianwangok.wav -D 1");
            
        ffc_inst.driver.set_status(LED_WIFI_CONNECTED);
                      
        mi.mpobj->open_service();
    #ifdef __ALI_MUSIC__  
        mi.maliobj->open_service();
    #endif                                                  
          
        auto_play(1);
    }

    LOG_PRT();
    wifi_configing = false;
    return NULL;
}

int wifi_config_start(void)
{
    pthread_mutex_lock(&wifi_config_lock);

    // cancle previous thread.
    if (wifi_configing) {
        swift_decode_shutdown(SWIFT_LINK);
        set_sa_config_interrupt(1);
        pthread_mutex_unlock(&wifi_config_lock);
        return 1;
    }

    // enter wifi config.
    if (pthread_create(&wifi_config_pthread, NULL, wifi_config_pthread_func, NULL) == -1) {
        printf("Create wifi config pthread failed: %s.\n", strerror(errno));
        pthread_mutex_unlock(&wifi_config_lock);
        return -1;
    } else {
        printf("Create wifi config pthread success!\n");
    }
    pthread_detach(wifi_config_pthread);

    pthread_mutex_unlock(&wifi_config_lock);

    return 0;

}

void keyEvAdjVolume(bool inc_vol)
{
	int vol;


    if(AOInf_I2S == ffc_inst.player.get_ad())
    {
        if(OK != audio_snd_ctl(0, "hw:1,0", "Master Volume", &vol))
            return;
     
        if(inc_vol) {
            vol += 6;
            
            if((0 < vol) && (143 > vol))
                vol = 143;
            
            if(vol >= 255)
                vol = 255; 
        }
        else
        {
            vol -= 6;
            
            if(vol < 143)
                vol = 0;
        }
                
        audio_snd_ctl(1, "hw:1,0", "Master Volume", &vol);

    }
    else
    {
        if(OK != audio_snd_ctl(0, "default", "head phone volume", &vol))
            return;

        LOG_PRT("oldvol=%d", vol);
        
        if(inc_vol) {
            vol += 6;
            if(vol >= 62) 
                vol = 62;
        }
        else
        {
            if(vol <= 6)
                vol = 0;
            else
                vol -= 6;
        }
                
        audio_snd_ctl(1, "default", "head phone volume", &vol);

    }

    LOG_PRT("adjust volume:%d",vol);   

}

void *key_ev_loop(void *arg)
{
	int key_fd, key1_fd;
	int max_fd = 0;
	int opt = 0;
    char ssid[128],pwd[128];
    int pre_mode, next_mode;
    
	fd_set rds;
	int ret;    
	struct input_event ev_key;
    //int pos;
    
    int long_press = 0;
    int ifly_flag = 0;
        
	key_fd = open("/dev/input/event0",O_RDONLY); //for keyboard
	if(key_fd <= 0)
	{
		key_fd = 0;
		LOG_PRT("open key event0 err!"); 
		exit(EXIT_FAILURE);
	}
	max_fd = key_fd;

	key1_fd = open("/dev/input/event2",O_RDONLY); //for power key
	if(key1_fd <= 0)
	{
	    LOG_PRT("open key event2 err!");
        exit(EXIT_FAILURE);
	}

	max_fd = max_fd > key1_fd ? max_fd : key1_fd;

	while(1)
	{

		FD_ZERO(&rds);
		FD_SET(key_fd, &rds);
		FD_SET(key1_fd, &rds);


		ret = select(max_fd+1,&rds,NULL,NULL,NULL);
		if (ret < 0)
		{
			if(errno == EINTR) continue;
			perror("select\n");
			exit(EXIT_FAILURE);
		}
		else if (ret == 0)
		{
		}
		else if(FD_ISSET(key_fd, &rds))
		{
			if(read(key_fd,&ev_key,sizeof(ev_key)) != sizeof(ev_key)) continue;
			if(EV_KEY != ev_key.type) continue;
            
            LOG_PRT("key code:%d value:%d",ev_key.code,ev_key.value);

            opt = KEY_UNVALID;
            
            if(wifi_configing)
            {
                if(103 != ev_key.code)//KEY_S_SACONFIG
                    continue;
            }
            
            if(0 == ev_key.value)//key up
            {
                
                if( long_press )
                {
                    long_press = 0;
                    LOG_PRT("key code:%d value:3",ev_key.code);
                    continue;
                }
                    
                
			    switch(ev_key.code)
    			{
    			case 102://sw6
   					opt = KEY_S_MODE;
    				break;
                    
    			case 114://sw3
  					opt = KEY_S_VOLDOWN;                    
    				break;

    			case 115://sw2
   					opt = KEY_S_VOLUP;
    				break;

    			case 28://sw5
                    opt = KEY_S_VOICE_END;
    				break;
                    
    			case 103://sw7
				    opt = KEY_S_SACONFIG;
    				break;
                    
    			case 139://sw4
				    opt = KEY_S_PLAYPAUSE;
    				break;                
                    
    			default:
    				break;

    			}
		                        
          }
          else if(2 == ev_key.value) //long press
          {
              long_press = 1;
              switch(ev_key.code)
              {
    			case 102://sw6
    				opt = KEY_S_AP;
    				break;
                    
    			case 114://sw3
                    opt = KEY_S_PREVIOUS;                    
    				break;

    			case 115://sw2
   					opt = KEY_S_NEXT;
    				break;

    			case 28://sw5
                    //opt = KEY_S_IFLY_START;   
                    long_press = 0;
    				break;
                    
    			case 103://sw7
				    opt = KEY_S_AP_STA;
    				break;
                    
    			case 139://sw4
				    opt = KEY_S_SETAD;
    				break;                
                    
    			default:
    				break;

    			}
          }
          else if(1 == ev_key.value) //key down
          {

              if(28 == ev_key.code)//sw5
                opt = KEY_S_VOICE_START;                

          }
          
                LOG_PRT("opt=%d====", opt);
                switch(opt)
        		{
        			case KEY_S_VOLUP:
        				
        				LOG_PRT("VOL++");
        				keyEvAdjVolume(1);
                        
                        if(MODE_PUSH == fa_obj.cur_mode)
                        {
                            mi.mpobj->commit(VOLUP);
                        } 
                      #ifdef __ALI_MUSIC__   
                        else if(MODE_ALIMUSIC == fa_obj.cur_mode)
                        {
                            mi.maliobj->sendmsg("player_volume_change", NULL);
                        } 
                      #endif

        				break;
        			case KEY_S_VOLDOWN :
        				
        				LOG_PRT("VOL--");
        				keyEvAdjVolume(0);

                        if(MODE_PUSH == fa_obj.cur_mode)
                        {
                            mi.mpobj->commit(VOLDOWN);
                        } 
                      #ifdef __ALI_MUSIC__   
                        else if(MODE_ALIMUSIC == fa_obj.cur_mode)
                        {
                            mi.maliobj->sendmsg("player_volume_change", NULL);
                        } 
                      #endif
                      
        				break;
        			case KEY_S_MODE:
                        next_mode = pre_mode = fa_obj.cur_mode;
                        next_mode++;
                        if(next_mode >= MODE_MAX)
                        {
                            next_mode = MODE_PUSH;
                        }
                        switch_audio_mode(pre_mode, next_mode);   

        				break;

        			case KEY_S_AP_STA:

                        auto_stop();
                        
                        ffc_inst.driver.set_status(LED_WIFI_CONNECTING);
                        ffc_inst.player.tone_play("/home/ad/aplianwang.mp3", 0);
          
                        if(STATION == ffc_inst.network.get_mode())
                        {
                            mi.mpobj->close_service(); 
                          #ifdef __ALI_MUSIC__  
                            mi.maliobj->close_service(); 
                          #endif  
                            ffc_inst.network.disconnect();
                            
                            getAPConfig(ssid,pwd,0);

                            sleep(2);
                            
                            ffc_inst.network.start_ap(ssid,pwd, 0);
                            
                            //open_service();

                        }

                        ret = user_run_softap(); //rev ssid and passwd
                        if(ret == 0)
                        {
                            ffc_inst.player.tone_play("/home/ad/mimajieshou.mp3", 0);
                            sleep(2);
                            b_ExtConfigSta = 1; //rev ssid and passwd successful, then go to connect
                            ap_sta_flag = 1;
                        }
                        else
                        {
                            ffc_inst.player.tone_play("/home/ad/jianchawangluo.mp3", 0);
                        }

                        break;

        			case KEY_S_AP:
                        
                        auto_stop();

                        ffc_inst.driver.set_status(LED_WIFI_DISCONNECT);

                        ffc_inst.player.tone_play("/home/ad/qiedaoap.mp3", 0);

                        mi.mpobj->close_service(); 
                      #ifdef __ALI_MUSIC__    
                        mi.maliobj->close_service();
                      #endif  
                        ffc_inst.network.disconnect();

                        getAPConfig(ssid,pwd,0);

                        sleep(2);
                        
                		ffc_inst.network.start_ap(ssid,pwd, 1);
                        
                        mi.mpobj->open_service(); //open dlna and airplay service

                        auto_play(0);
                        
        				break;

                    case KEY_S_VOICE_START:

                      #ifdef __ALI_MUSIC__

                        if(MODE_ALIMUSIC != fa_obj.cur_mode)
                            continue;

                        if(0 != ffc_inst.network.get_status())
                        {
                            ffc_inst.player.tone_play("/home/ad/jianchawangluo.mp3", 0);
                            continue;
                        }
                        
                        mi.maliobj->vptr->stop();
                        mi.maliobj->vptr->record(); //ALI goto record
                        
                      #else //__IFLY_VOICE__    

                        if(MODE_SPEECH != fa_obj.cur_mode)
                            continue;

                        LOG_PRT();
                        
                        if(OK != ffc_inst.network.get_status())
                        {
                            ffc_inst.player.tone_play("/home/ad/jianchawangluo.mp3", 0);
                            continue;
                        }

                        ifly_flag = 1;
                        
                        mi.miflyobj->vptr->stop();
                        mi.miflyobj->vptr->record(); //IFLY go to record
                        
                      #endif    
                        
                        break;
                        
                    case KEY_S_VOICE_END:

                      #ifdef __ALI_MUSIC__
                        
                        if(MODE_ALIMUSIC == fa_obj.cur_mode)
                        {
                            mi.maliobj->vptr->stop();
                            
                            ret = mi.maliobj->analyze_pcm(); //ALI analyze record file
                            if(0 == ret)                            
                            {
                                ffc_inst.player.tone_play("/home/ad/searching.mp3", 0);
                            }                                
                            else if(1 == ret)      
                            {
                                ffc_inst.player.tone_play("/home/voice/28.mp3", 0);        
                            }                                                    
                            else                            
                            {
                                ffc_inst.player.tone_play("/home/voice/26.mp3", 0);     
                                mi.maliobj->sendmsg("play_done", NULL);
                            }                                                        
                        }

                      #else//__IFLY_VOICE__
                        
                        if( (MODE_SPEECH == fa_obj.cur_mode) && ifly_flag )
                        {
                            ifly_flag = 0;
                            
                            mi.miflyobj->vptr->stop();
                            mi.miflyobj->vptr->play(NULL); //IFLY analyze record file and auto play
                        }

                      #endif
                      #ifdef __SIMPLE_PLAY__
                        
                        else if(MODE_SIMPLE == fa_obj.cur_mode)
                        {
                            if(++mi.msimpleobj->ch >= XI_MAX)
                                mi.msimpleobj->ch = 0;

                            LOG_PRT("channel:%d", mi.msimpleobj->ch);
                            if( OK == mi.msimpleobj->url_parse(mi.msimpleobj->radio_src[mi.msimpleobj->ch]) )
                            {
                                mi.msimpleobj->vptr->stop();
                                mi.msimpleobj->vptr->play(NULL); //  SIMPLE mode, play next channel
                            }                                
                            else
                                ffc_inst.player.tone_play("/home/ad/searchfail.mp3", 0);

                        }

                      #endif
                      
                        break;
                        
                    case KEY_S_SACONFIG:

                    wifi_config_start();

                    break;

                    case KEY_S_PREVIOUS:
                        if(MODE_PUSH == fa_obj.cur_mode)
                        {
                            mi.mpobj->commit(PREVIOUS);
                        }                        
                        else if(MODE_LOCAL == fa_obj.cur_mode)
                        {
                            mi.mlcobj->go_play_prev();
                        }
                      #ifdef __IFLY_VOICE__                        
                        else if(MODE_SPEECH == fa_obj.cur_mode)
                        {
                            mi.miflyobj->go_play_prev();
                        }
                      #endif
                      #ifdef __SIMPLE_PLAY__
                        else if(MODE_SIMPLE == fa_obj.cur_mode)
                        {
                            mi.msimpleobj->go_play_prev();
                        }
                      #endif  
                      #ifdef __SUPPORT_BT__
                        else if(MODE_BT == fa_obj.cur_mode)
                        {
                            user_bt_prev();
                        }                  
                      #endif
                      #ifdef __ALI_MUSIC__   
                        else if(MODE_ALIMUSIC == fa_obj.cur_mode)
                        {
                            mi.maliobj->sendmsg("button", "next_channel");
                        } 
                      #endif 
                      
                        break;  

                    case KEY_S_NEXT:
                        if(MODE_PUSH == fa_obj.cur_mode)
                        {
                            mi.mpobj->commit(NEXT);
                        }                        
                        else if(MODE_LOCAL == fa_obj.cur_mode)
                        {
                            mi.mlcobj->go_play_next();
                        }
                      #ifdef __IFLY_VOICE__    
                        else if(MODE_SPEECH == fa_obj.cur_mode)
                        {
                            mi.miflyobj->go_play_next();
                        }
                      #endif  
                      #ifdef __SIMPLE_PLAY__
                        else if(MODE_SIMPLE == fa_obj.cur_mode)
                        {
                            mi.msimpleobj->go_play_next();
                        }
                      #endif  
                      #ifdef __SUPPORT_BT__
                        else if(MODE_BT == fa_obj.cur_mode)
                        {
                            user_bt_next();
                        }
                      #endif
                      #ifdef __ALI_MUSIC__   
                        else if(MODE_ALIMUSIC == fa_obj.cur_mode)
                        {
                            mi.maliobj->sendmsg("button", "next");
                        } 
                      #endif   
                      
                        break;  

                    case KEY_S_PLAYPAUSE:
                        auto_play_pause();
                        //ffc_inst.player.tone_play("/home/ad/searchfail.mp3");
                        //pos = 60;
                        //ffc_inst.player.adplayer(AD_SEEK, &pos);
                        
                        break;

                    case KEY_S_SETAD:                                               
                        ffc_inst.player.select_ad();//switchAdout();          
                        //mi.maliobj->sendmsg("button", "love_audio");
                        //pos = 30;
                        //ffc_inst.player.adplayer(AD_SEEK, &pos);
                        
                        //ffc_inst.player.adplayer(0, "http://toys.hivoice.cn/tb/M00/00/14/Cg0kJ1fOnuKASIMuABtAjZ5UEyI867.mp3");
                        break;
                        
        			default:
        				break;
        		}          
		}
        else if(FD_ISSET(key1_fd, &rds))//for power key
		{
			if(read(key1_fd,&ev_key,sizeof(ev_key)) != sizeof(ev_key)) continue;
			if(EV_KEY != ev_key.type) continue;
                        
            switch(ev_key.code)
            {
                case 108:
                    if(1 == ev_key.value)   //Down
                    {
                        gettimeofday(&powerKey_Down_t,NULL);
                        b_powerKey_Down = 1;
                    
                        press_power_off();
                    
                    }
                    else if(0 == ev_key.value)  //Up
                    {
                        b_powerKey_Down = 0;
                    }

                    break;

    			default:
    				break;
                
            }
                
		}        

		usleep(1000);
        
	}
    
}

void init_hw_keypad(void)
{
	int ret;
	pthread_t tid;
	ret = pthread_create(&tid, NULL, &key_ev_loop, NULL);
	if(ret != 0)
	{
		perror("pthread_create tid1 failed\n");
		return;
	}
}



int audio_snd_ctl(int dir, const char *card, const char *name, int *val)
{
    
	int err, ret = OK;
	static snd_ctl_t *handle = NULL;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_value_t *control;

    //LOG_PRT("dir:%d  card:%s  name:%s  val:%d", dir,card,name,*val);
    pthread_mutex_lock(&alsa_mutex_lock);

	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_value_alloca(&control);

	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);/* default */

	snd_ctl_elem_id_set_name(id, name);

	if ((err = snd_ctl_open(&handle, card, 0)) < 0) 
	{
		fprintf(stderr,"Control %s open error: %s\n", card, snd_strerror(err));
        pthread_mutex_unlock(&alsa_mutex_lock);
		return ERROR;
	}

	snd_ctl_elem_value_set_id(control, id);

    if(dir)
    {
        //LOG_PRT("snd_ctl_elem_value_set_integer:%d", *val);
        snd_ctl_elem_value_set_integer(control, 0, *val);
        if ((err = snd_ctl_elem_write(handle, control)) < 0)
        {
            fprintf(stderr,"Control %s element write error: %s\n", card, snd_strerror(err));
            ret = ERROR;
        }
    }
    else
    {
        if (!snd_ctl_elem_read(handle, control))
        {
            *val = snd_ctl_elem_value_get_integer(control, 0);
            //LOG_PRT("snd_ctl_elem_value_get_integer:%d", *val);
        }
        else
            ret = ERROR;
    }

	snd_ctl_close(handle);
	handle = NULL;
    
    pthread_mutex_unlock(&alsa_mutex_lock);

	return ret;
}



void audio_set_mute(int onoff)//flag 0:mute 1:unmute
{
    LOG_PRT("audio_set_mute-----%d", onoff);

    if(AOInf_I2S == ffc_inst.player.get_ad())
    {
        audio_snd_ctl(1, "hw:1", "Master Switch", &onoff);
    }
    else
    {
        audio_snd_ctl(1, "hw:0", "dac: right mute", &onoff);
        audio_snd_ctl(1, "hw:0", "dac: left mute", &onoff);
    }

}

void sig_handler(int sig)
{
    s_file_context fc;
        
	switch(sig)
	{

	    case SIGUSR2: //remove sd card
		
			LOG_PRT("remove sd-card------");

            if(MODE_LOCAL == fa_obj.cur_mode)
                mi.mlcobj->vptr->stop();

            mi.mlcobj->unplug();

			break;
		
        
	    case SIGUSR1:  //insert sd card
		
			LOG_PRT("insert sd-card------");
            
            mi.mlcobj->insert();
            
            sleep(1);

            if(MODE_LOCAL == fa_obj.cur_mode)
                mi.mlcobj->vptr->play(NULL);
            
        #ifdef __ALI_MUSIC__
            system("mkdir /mnt/sdcard/CUCKOO");
        #endif
        
			break;
            
        case SIGALRM://seek of dlna
    	
    		LOG_PRT("--------SIGALRM get!------~~");
            
    		if(0 != sem_trywait(&(mi.mpobj->mp_sem_sk)))
    		{
    			LOG_PRT("mp_sem_sk is busy");
    			return ;
    		}

        #ifdef __MULTI_ROOM__
            server_notice_msg_handle(ACT_PLAYER_SEEK);
        #endif

            //mi.mpobj->vptr->stop();
            mi.mpobj->dlna->stop();

            mi.mpobj->vptr->get_file_attr(&fc);

            LOG_PRT("fc.url:%s, seektime:%d", fc.url, fc.seek_time);
            
    		mi.mpobj->vptr->seek(fc.seek_time);
            mi.mpobj->dlna->play(fc.url); 
            
    		sem_post(&(mi.mpobj->mp_sem_sk));

    		break;		
            
        default:
    		break;
        
	}
    
}

void check_to_play_next()
{
    if(MODE_LOCAL == fa_obj.cur_mode)
    {            
        if(OK == mi.mlcobj->check_next())
            mi.mlcobj->go_play_next();
    }
    #ifdef __IFLY_VOICE__
    else if(MODE_SPEECH == fa_obj.cur_mode)
    {            
        if(OK == mi.miflyobj->check_next())
            mi.miflyobj->go_play_next();
    }
    #endif
    #ifdef __SIMPLE_PLAY__
    else if(MODE_SIMPLE == fa_obj.cur_mode)
    {            
        if(OK == mi.msimpleobj->check_next())
            mi.msimpleobj->go_play_next();
    }
    #endif
    #ifdef __ALI_MUSIC__
    else if(MODE_ALIMUSIC == fa_obj.cur_mode)
    {            
        if(OK == mi.maliobj->check_next())
            mi.maliobj->go_play_next();
    }
    #endif

}

void ext_sta_connect()
{
    char ssid[128],pwd[128],encryptType[32];

    
    if(!b_ExtConfigSta) 
        return;  //don't connect

    LOG_PRT("ext_sta_connect--");
    
    b_ExtConfigSta = 0;

	//STA
    getSTAConfig(encryptType,ssid,pwd);


    mi.mpobj->close_service();
#ifdef __ALI_MUSIC__
    mi.maliobj->close_service();    
#endif
    sleep(2);
    ffc_inst.network.stop_ap();
    sleep(2);
	ffc_inst.network.connect(encryptType,ssid,pwd);

    //STA Success.

    int i = 0;

    while(i < 60) //60s,if connection is fail,then quit
    {

        if(NULL != get_local_ip("wlan0")) 
        {
            if(ap_sta_flag)
            {
                ap_sta_flag = 0;
                user_start_udp();
            }
    
            system("/home/tinyplay /home/ad/lianwangok.wav");
            system("/home/tinyplay /home/ad/lianwangok.wav -D 1");

            ffc_inst.driver.set_status(LED_WIFI_CONNECTED);
                        
            mi.mpobj->open_service(); //open dlna and airplay
            
        #ifdef __ALI_MUSIC__
            if(NULL != mi.maliobj)
                mi.maliobj->open_service();      
        #endif

            auto_play(1);
            
            return ;

        }  
        
        sleep(1);
        
        i++;
            
    }

    ffc_inst.player.tone_play("/home/ad/jianchawangluo.mp3", 0); //connection is fail
    auto_play(0);
    
}

void init_mac_addr(void)
{
    FILE *fp;
    int i;
    char buf[32];
    char hwaddr[18] = {0};
    char cmd[256]={0};
        
    #define MAC_ADRESS_FILE_NAME   "/tmp/conf/mac-address.conf"

    if((fp=fopen("/tmp/conf/config.dat","r")) != NULL)
    {
        for(i=0;i<10;i++)
        {
            memset(buf,0x0,sizeof(buf));
            if(fgets(buf,sizeof(buf),fp) != NULL)
            {
                if(i==9)   //line 10 
                {
                    if(!strncmp("FF:FF:FF:FF:FF:FF",buf,17))
                    {          
                        if(OK == getHwaddr("wlan0", hwaddr))
                        {
                            LOG_PRT("hwaddr:%s", hwaddr);
                            
                            sprintf(cmd,"echo %s > %s",hwaddr, MAC_ADRESS_FILE_NAME);
                            system(cmd);
                            
                            sprintf(cmd,"/usr/sbin/save.sh 7 '%s'",hwaddr);
                            system(cmd);                            
                        }  
                    }
                    else
                    {          
                        //LOG_PRT("buf=%s len=%d", buf,strlen(buf));
                        strncpy(hwaddr, buf, 17);
                        hwaddr[17] = '\0';
                        sprintf(cmd,"echo %s > %s",hwaddr, MAC_ADRESS_FILE_NAME);
                        system(cmd);
                    }                 
                }                       
            }
        }
        fclose(fp);
    }
    
}


int main()
{

    int ret;
    
    printf("fireair start--------------\n");

    pthread_mutex_init(&fa_mutex_lock, NULL);
    pthread_mutex_init(&alsa_mutex_lock, NULL);

	//signal for sd card and dlna seek
    signal(SIGUSR1, sig_handler); 
    signal(SIGUSR2, sig_handler);
    signal(SIGALRM, sig_handler);

    //factory
    factory_init(&ffc_inst);
    
    //creat user modes   
    fa_obj.new_object(&mi, CLASS_PUSH); //creat push mode     
  
    fa_obj.new_object(&mi, CLASS_LINEIN_BT);   
    
    fa_obj.new_object(&mi, CLASS_LOCAL); //creat local play mode      
    
#ifdef __IFLY_VOICE__
    fa_obj.new_object(&mi, CLASS_SPEECH);     
#endif

#ifdef __SIMPLE_PLAY__
    fa_obj.new_object(&mi, CLASS_SIMPLE);       
#endif

#ifdef __ALI_MUSIC__
    fa_obj.new_object(&mi, CLASS_ALIMUSIC);  //creat ali play mode    
    system("killall vendor");
    system("rm /tmp/doug.pid");
#endif

    //fa_obj.cur_mode = MODE_ALIMUSIC;
    //mi.maliobj->outof_dormancy();
    
    system("echo 1 > /sys/class/gpio/gpio0/value");//open usb power
      
    init_mac_addr(); //init mac addr 
	
    ffc_inst.driver.led_init(); //about led contrl
 
    ret = ffc_inst.network.init(); 
    if(HOST_AP == ret)
    {
        if(NULL != mi.mpobj)
            mi.mpobj->open_service();
    }
    else if(STATION == ret)
    {
        if(NULL != mi.mpobj)
            mi.mpobj->open_service();

      #ifdef __ALI_MUSIC__
        if(NULL != mi.maliobj)
        {
            mi.maliobj->notify_app();
            mi.maliobj->open_service();                  
        }            
      #endif  
    }

#ifdef __ALI_MUSIC__ 
    switch_audio_mode(MODE_MAX, MODE_ALIMUSIC);   
#else
    switch_audio_mode(MODE_MAX, MODE_PUSH);
#endif

    init_hw_keypad();    

#ifdef __ANROID_APP_LINK__   // in MULTI_ROOM mode,  App link c600 
    LOG_PRT("wait to connect android app...\n");
    pthread_t tid; 
    ret = pthread_create(&tid, NULL, app_connection_creat_pthread, NULL); 
    if(ret != 0)
    {
        LOG_PRT("creat app_connection_creat_pthread fail!\n");
    }
#endif

    while(1)
    {         
        sleep(1);

        ext_sta_connect(); 
        check_to_play_next(); 
    }
    
    return 0;
}


