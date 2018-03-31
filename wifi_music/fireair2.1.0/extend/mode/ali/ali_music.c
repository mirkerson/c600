
#ifdef __ALI_MUSIC__


#include <stdio.h>
#include "ali_music.h"
#include <sys/time.h>
#include <json.h>
#include "hub.h"
#include <errno.h>
#include <math.h> 
#include <stdbool.h>
#include <fcntl.h>

#include "ad_type.h"
#include "ali_music.h"
 
static mali *mali_obj;
static int ali_sleep = 0;
static pthread_mutex_t ali_mutex_lock;
static int ali_inited;


struct   hub_ctx ctx;

int ali_music_init(void);
int ali_music_exit(void);
int ali_notification_event(char * method ,char * name);


void ali_into_dormancy(void)
{
    ali_sleep = 1;
}

void ali_outof_dormancy(void)
{
    ali_sleep = 0;
}

static void ali_setVolume(int vol)
{
    int new_vol;
    
    pthread_mutex_lock(&ali_mutex_lock);

    if(AOInf_I2S == ffc_inst.player.get_ad())
    {
        new_vol = floor(vol*1.13); 
        if(0 != new_vol)
            new_vol += 142;
        
        if(OK != audio_snd_ctl(1, "hw:1,0", "Master Volume", &new_vol))
            LOG_PRT("error occur!");
    }

    else
    {
        new_vol = ceil(vol*0.62);
    
        if(OK != audio_snd_ctl(1, "default", "head phone volume", &new_vol))
            LOG_PRT("error occur!");
    }

    pthread_mutex_unlock(&ali_mutex_lock);


}

static int ali_getVolume(void)
{
	int old_vol, new_vol = 0;

    pthread_mutex_lock(&ali_mutex_lock);
	
	//old_vol = audio_get_vol();

    if(AOInf_I2S == ffc_inst.player.get_ad())
    {
        if(OK == audio_snd_ctl(0, "hw:1,0", "Master Volume", &old_vol))
        {
            old_vol -= 142;
            if(old_vol < 0)
                old_vol = 0;
            new_vol = ceil(old_vol/1.13);
        }        
    }
    else
    {
        if(OK == audio_snd_ctl(0, "default", "head phone volume", &old_vol))
        {
            new_vol = floor(old_vol/0.62);
        }
    }
    
	pthread_mutex_unlock(&ali_mutex_lock);


	return new_vol;
}

mali* ali_init(void)
{
    
    mali_obj = (mali *)malloc(sizeof(mali));
    if(NULL == mali_obj)
    {
        return NULL;
    }    

    mali_obj->vptr =  &alimode;
    mali_obj->open_service =  ali_music_init;
    mali_obj->close_service =  ali_music_exit;
    //mali_obj->set_volume =  ali_setVolume;
    //mali_obj->get_volume =  ali_getVolume;
    mali_obj->into_dormancy =  ali_into_dormancy; 
    mali_obj->outof_dormancy =  ali_outof_dormancy;
    mali_obj->simple_config =  ali_simple_config; //simple config network
    mali_obj->sendmsg =  ali_notification_event;
    mali_obj->check_next =  ali_check_next;
    mali_obj->go_play_next =  ali_go_play_next;
    mali_obj->analyze_pcm =  ali_analyze;  //ali mode voice  analyze
    mali_obj->notify_app =  ali_notify_app;

    ali_sleep = 1;

    pthread_mutex_init(&ali_mutex_lock, NULL);

    return mali_obj;
}

void ali_exit(void)
{

    mali_obj->vptr->unit();
    
    pthread_mutex_destroy(&ali_mutex_lock);

    free(mali_obj);
    mali_obj = NULL;    
}

int ali_notification_event(char * method ,char * name)
{
    json_object *p_root = NULL, * p_params = NULL;
	json_object *p_error = NULL;     
    char *p_json_str = NULL;
	char uri[256] = {0};
	int  vol;
    s_file_context fc;
    bool sta;
    int  ret = ERROR;
    char cmd[256];

    LOG_PRT();
    
    if(!ali_inited)
    {   
        LOG_PRT("ali not inited!");
        return ERROR;
    }
        
    
	p_root   = json_object_new_object();
	p_params = json_object_new_object();
 
    if(p_root&& p_params)
    {
        if(!strcmp(method, "button"))
        {
		    json_object_object_add(p_root, "jsonrpc", json_object_new_string("2.0"));
			json_object_object_add(p_root, "method", json_object_new_string("button"));
			json_object_object_add(p_root, "params", p_params);
			if(!strcmp(name, "next"))
			{
	 			json_object_object_add(p_params, "name", json_object_new_string("next"));
	 			p_json_str = strdup(json_object_get_string(p_root));
	 			memcpy(cmd,p_json_str,strlen(p_json_str));
	 		}
	 		else if(!strcmp(name, "next_channel"))
	 		{
	 		    json_object_object_add(p_params, "name", json_object_new_string("next_channel"));
	  			p_json_str = strdup(json_object_get_string(p_root));
	 			memcpy(cmd,p_json_str,strlen(p_json_str));

			}
			else if(!strcmp(name, "previous"))
			{
	 		    json_object_object_add(p_params, "name", json_object_new_string("previous"));
	  			p_json_str = strdup(json_object_get_string(p_root));
	 			memcpy(cmd,p_json_str,strlen(p_json_str));

			}
			else if(!strcmp(name, "love_audio"))
			{
	 		    json_object_object_add(p_params, "name", json_object_new_string("love_audio"));

                mali_obj->vptr->get_file_attr(&fc);                
				strcpy(uri, fc.url);

                json_object_object_add(p_params, "uri", json_object_new_string(uri));
	  			p_json_str = strdup(json_object_get_string(p_root));
	 			memcpy(cmd,p_json_str,strlen(p_json_str));

			}
			else if(!strcmp(name, "switch_mode"))
			{
	 		    json_object_object_add(p_params, "name", json_object_new_string("switch_mode"));
				if(0 == ali_sleep)
				{
				     sta = true;
				}
				else
				{
                     sta = false;
				}
				
				json_object_object_add(p_params, "attach", json_object_new_boolean(sta));
	  			p_json_str = strdup(json_object_get_string(p_root));
	 			memcpy(cmd,p_json_str,strlen(p_json_str));

			}
	    }
        else if(!strcmp(method, "play_done"))
        {
            json_object_object_add(p_root, "jsonrpc", json_object_new_string("2.0"));
			json_object_object_add(p_root, "method", json_object_new_string("play_done"));
			json_object_object_add(p_root, "params", p_params);

            mali_obj->vptr->get_file_attr(&fc);                
			strcpy(uri, fc.url);

            json_object_object_add(p_params, "uri", json_object_new_string(uri));
			if(0)
			{
    			json_object_object_add(p_params, "status", json_object_new_int(1));
				json_object_object_add(p_params, "error",  p_error);
			    json_object_object_add(p_error,  "code",   json_object_new_int(403));

			}
			else
			{
    			json_object_object_add(p_params, "status", json_object_new_int(0));
			}
			p_json_str = strdup(json_object_get_string(p_root));
	 		memcpy(cmd,p_json_str,strlen(p_json_str));

		}
        else if(!strcmp(method, "player_volume_change"))
        {
            json_object_object_add(p_root, "jsonrpc", json_object_new_string("2.0"));
			json_object_object_add(p_root, "method", json_object_new_string("player_volume_change"));
			json_object_object_add(p_root, "params", p_params);

			vol = ali_getVolume();
			
			//LOG_DEBUG("vol =%d",vol);
			json_object_object_add(p_params, "volume", json_object_new_int(vol));
			p_json_str = strdup(json_object_get_string(p_root));
	 		memcpy(cmd,p_json_str,strlen(p_json_str));
			
		}
        else if(!strcmp(method, "player_state_change"))
        {
            json_object_object_add(p_root,  "jsonrpc", json_object_new_string("2.0"));
			json_object_object_add(p_root,  "method", json_object_new_string("player_state_change"));
			json_object_object_add(p_root,  "params", p_params);


			if(mali_obj->vptr->get_status() == IS_STOP)
			{
        		 json_object_object_add(p_params, "state", json_object_new_int(2)); 
    		}
    		else if(mali_obj->vptr->get_status() == IS_PAUSE)
    		{
    		     json_object_object_add(p_params, "state", json_object_new_int(1));
    	    }
    	    else
    	    {
        		 json_object_object_add(p_params, "state", json_object_new_int(0));                 
    		}

			p_json_str = strdup(json_object_get_string(p_root));
	 		memcpy(cmd,p_json_str,strlen(p_json_str));

		}
        else if(!strcmp(method, "setup_wifi_state_change"))
        {
            json_object_object_add(p_root, "jsonrpc", json_object_new_string("2.0"));
			json_object_object_add(p_root, "method", json_object_new_string("setup_wifi_state_change"));
			json_object_object_add(p_root, "params", p_params);

			json_object_object_add(p_params, "state", json_object_new_int(1));
			p_json_str = strdup(json_object_get_string(p_root));
	 		memcpy(cmd,p_json_str,strlen(p_json_str));
		}

        LOG_PRT();
        ret = hub_send(&ctx, cmd, strlen(cmd));
		if(ret < 0)        
			perror("hub_send error");
		
        FREE_MEM(p_json_str);
        ret = OK;
        
	}	
	
	printf("notif send event %s\n",cmd);

	FREE_JSON_OBJ(p_root);    
    FREE_JSON_OBJ(p_params);
    
	return ret;
    
}

int read_mac_from_file(char *mac) 
{
	int ret = access(MAC_ADRESS_FILE_NAME, 0);
	if (ret < 0)
	    return ERROR;
    
	FILE *f = fopen(MAC_ADRESS_FILE_NAME, "r");
	fscanf(f, "%17s", mac);
	fclose(f);
    
	return OK;
}

void print_usage() 
{
	char *usage = "Usage: atalk-cli [mac-address]\n\n" \
	            "Note: A single line MAC address with plain text format can be specified in `./mac-address.conf`. "\
	            "Feel free to override the configuration file with the command line argument.";
	puts("No mac address provided.\n\n");
	puts(usage);
}


void * ali_music_thread(void *arg)
{
	int  vol;
    int  id,volume,ret;
	char cmd[640]        = {0};
	char resp[512]       = {0};
	char mac_address[20] = {0};
	
	
	struct timespec ts = {0, 2000000};
	char *method = NULL, *uri = NULL;
	json_object *recv_msg = NULL , * p_method = NULL , * p_params = NULL;
	json_object * p_uri = NULL, * p_id= NULL,*p_volume= NULL;
    int sta;

	sleep(1);
	
	hub_init(&ctx, HUB_HOST);
	
    if(OK != read_mac_from_file(mac_address)) 
    {                    
		print_usage();// TODO no mac address found, print usage
		return NULL;
	}
	
	
	printf("Atalk CLI is running with MAC address: %s\n\n", mac_address);
	system("/tmp/atalk/vendor /tmp/atalk/prodconf.json   > /dev/null & ");
    usleep(1000*200);


	//if(!ali_sleep)
    //    ffc_inst.player.tone_play("/home/ad/26.mp3" );
    
	while (1) 
    {  

	    memset(cmd, 0, sizeof(cmd));
		hub_recv(&ctx, cmd, sizeof(cmd));

		printf("\n---------------------------------------------\n");
		printf("Recv: %s\n", cmd);
		printf("---------------------------------------------\n\n");

	 
		recv_msg = json_tokener_parse(cmd);
		if(recv_msg)
        {
            p_method = json_object_object_get(recv_msg, "method");
			p_id     = json_object_object_get(recv_msg, "id");
			
			if(p_id)
            {
                id = json_object_get_int(p_id);
				printf("\n----id = %d------\n",id);
			}
			
			memset(resp, 0, sizeof(resp));
			if(p_method)
            {
                method = json_object_get_string(p_method);
			    if ( !strcmp(method, "play") && !ali_sleep )
                {
				    p_params = json_object_object_get(recv_msg, "params");

					p_uri = json_object_object_get(p_params, "uri");
					if(p_uri)
                    {
					    uri = json_object_get_string(p_uri);
					    if(uri)
                        {
						    //printf("----uri:%s\n\n",uri);

                            mali_obj->vptr->stop();
                            mali_obj->vptr->play(uri);
                        }
                            
                    }
                    else
                    {
					    LOG_PRT("url NULL");
						continue;				 
                    }
                                        
			    }
                
                else if( !strcmp(method, "stop") && !ali_sleep )
                {
				  
                    printf("\n-------------stop--------------\n");

					mali_obj->vptr->stop();					  

				}
                
                else if(!strcmp(method, "get_mac_address")) 
                {   
                    //sleep(2);
					sprintf(resp, "{\"jsonrpc\":\"2.0\", \"result\":{\"mac_address\":\"%s\"}, \"id\":\"%d\"}", mac_address, id);
      				ret = hub_send(&ctx, resp, strlen(resp));
                    if (ret < 0) 
                    {
                        perror("Fail to hub send.\n");
                        perror(strerror(errno));
                    }
                    else
                    {
                        LOG_PRT("get_mac_address ret = %d",ret);
					}
					usleep(1000*380);
				}
                
                else if(!strcmp(method, "is_attaching"))
                {
				    sprintf(resp, "{\"jsonrpc\":\"2.0\", \"result\":{\"attach\":\"true\"}, \"id\":\"%d\"}", id);
					ret = hub_send(&ctx, resp, strlen(resp));
					if (ret < 0)
                    {
                        perror("Fail to hub send.\n");
                    	perror(strerror(errno));
                    }
                    else
                    {
                        LOG_PRT("is_attaching ret = %d",ret);
				    }

				}

                else if(!strcmp(method, "get_setup_wifi_state"))
                {
				    sprintf(resp, "{\"jsonrpc\":\"2.0\", \"result\":{\"state\":1}, \"id\":\"%d\"}", id);
					ret = hub_send(&ctx, resp, strlen(resp));
					if (ret < 0)
                    {
                        perror("Fail to hub send.\n");
                        perror(strerror(errno));
                    }
                    else
                    {
                        LOG_PRT("get_setup_wifi_state ret = %d",ret);
					}
					usleep(1000*280);
				}

                else if( !strcmp(method, "play_voice_prompt") && !ali_sleep )
                {
				    printf("\n-----------play_voice_prompt----------\n");
			
                    p_params = json_object_object_get(recv_msg, "params");
					p_uri    = json_object_object_get(p_params, "uri");
						  
					if(p_uri)
                    {
					    uri = json_object_get_string(p_uri);
						if(uri)
                        {
							LOG_PRT("----uri:%s---",uri);

							if(NULL != strstr(uri,"34.mp3"))
                            {
							    LOG_PRT("--network busy--");
                            }

                            ffc_inst.player.tone_play(uri, 0);									  
						}
						  	  
                    }
                    else
                    {
						LOG_PRT("p_url NULL\n");
					    continue;				 
					}					  
	
			    }
                
                else if( !strcmp(method, "resume") && !ali_sleep )
                {
					printf("\n-----------resume----------\n");
                    mali_obj->vptr->play(NULL);
			    
			    }

                else if( !strcmp(method, "pause") && !ali_sleep )
                {
				    printf("\n-----------pause----------\n");
			  		mali_obj->vptr->pause();					  
			    }

                else if(!strcmp(method, "pause_toggle"))
                {
				    printf("\n-----------pause_toggle----------\n");		  
			    }

                else if(!strcmp(method, "net_state_change"))
                {
                    p_params = json_object_object_get(recv_msg, "params");              
                    p_uri  = json_object_object_get(p_params, "state");
                    sta = json_object_get_int(p_uri);

				    printf("\n-----------net_state_change:%d----------\n", sta);

                    if(!sta)
                    {
                        mali_obj->vptr->stop();
                        if(0 != ali_into_offline_mode())
                            mali_obj->vptr->play(NULL);
                    }
                    else
                    {
                        mali_obj->vptr->stop();
                        ali_outof_offline_mode();
                    }
                    
				}
                else if(!strcmp(method, "play_cached_items"))
                {
                    p_params = json_object_object_get(recv_msg, "params");              
                    p_uri  = json_object_object_get(p_params, "state");
                    sta = json_object_get_int(p_uri);

				    printf("\n-----------play_cached_items:%d----------\n", sta); 

                    if(sta)
                    {
                        mali_obj->vptr->stop();
                        if(0 != ali_into_offline_mode())
                            mali_obj->vptr->play(NULL);
                    }
                    else
                    {
                        mali_obj->vptr->stop();
                        ali_outof_offline_mode();
                    }
                        
				}

                else if(!strcmp(method, "get_volume"))
                {
					  nanosleep(&ts, NULL);
					  vol = ali_getVolume();
					 
            		  LOG_PRT("vol = %d",vol);
                      
					  sprintf(resp, "{\"jsonrpc\":\"2.0\", \"result\":{\"volume\":\"%d\"}, \"id\":\"%d\"}",vol,id);
					  hub_send(&ctx, resp, strlen(resp));
				}

                else if( !strcmp(method, "set_volume") && !ali_sleep )
                {
				    p_params = json_object_object_get(recv_msg, "params");
						  	 
					printf("\n-----------set_volume----------\n");
					p_volume = json_object_object_get(p_params, "volume");
					if(p_volume)
                    {
					    volume = json_object_get_int(p_volume);
                        LOG_PRT("volume = %d",volume);

                        ali_setVolume(volume);
					}
                    else
                    {
					    LOG_PRT("p_volume NULL");
						continue;				 
					}
					   
						
                }

			    FREE_JSON_OBJ(p_method);
                FREE_JSON_OBJ(p_params);
				FREE_JSON_OBJ(p_uri);
				FREE_JSON_OBJ(p_id);
               
			}
            else
            {
                LOG_PRT("method NULL!");
				continue;	
		    }			

		}
        else
        {
			printf("recv_msg NULL\n");
			continue;
	    }
			
		FREE_JSON_OBJ(recv_msg);
		
	}
	
	hub_destroy(&ctx);

}

int ali_music_exit(void)
{
    LOG_PRT();
    
	system("killall vendor");
	system("rm /tmp/doug.pid");

    ali_inited = 0;    
}

int ali_music_init(void)
{
 	int        ret;
	pthread_t  tid_ali = 0;
			
	LOG_PRT("ali_music_init");

    if(ali_inited)
        ali_music_exit();
    
	doSysCommand("date --set \"2016-01-15 15:20\"");
	
	ret = pthread_create(&tid_ali, NULL, &ali_music_thread,NULL);  
	if(ret != 0)
	{  
		LOG_PRT("pthread_create tid_ali failed");	
		return ret;
	}

    ali_inited = 1;

}

#endif

