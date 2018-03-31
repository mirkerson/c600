
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>  
#include <memory.h>  
#include <signal.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <arpa/inet.h>  
#include <netdb.h>  
#include <netinet/in.h>  
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>


#include <alsa/asoundlib.h>   
#include <alsa/pcm.h>

#include "wifi.h"
#include "api.h"

#include "swift_link.h"
int swift_link_connect_ap(void)
{
    swift_state_t ret;
    swift_link_result_t swift_result;
    param_info_t param_info;
    memset(&swift_result, 0, sizeof(swift_link_result_t));
    memset(&param_info, 0, sizeof(param_info_t));
    param_info.adt_timeout = 60;
    param_info.smt_timeout = 60;
    param_info.dev_info.devicename = "hw:0,0";
    param_info.dev_info.rate = 16000;
    param_info.dev_info.channels = 1;
    param_info.dev_info.bits = SND_PCM_FORMAT_S16_LE;

    int on=1, off=0;
    audio_snd_ctl(1, "default", "ADC mixer mute for FML" , &off);
    audio_snd_ctl(1, "default", "ADC mixer mute for FMR" , &off);
    audio_snd_ctl(1, "default", "ADC mixer mute for mic" , &on);
    audio_snd_ctl(1, "default", "ADC MIC Boost AMP en" , &on);

    swift_decode_reset(SWIFT_LINK);
    ret = swift_decode_start(&swift_result, param_info, SWIFT_LINK);
    if(ret == DEC_ADT_NORM || ret == DEC_SMT_NORM){
        ffc_inst.network.connect(key_mgmt_str[swift_result.encrypt_type], swift_result.ssid, swift_result.passwd);
    }else{
        printf("swift link output: error!!! \n");
        return ERROR;
    }
    int i = 0;
    while(i < 60){
        if(get_sa_config_interrupt() > 0) {
            set_sa_config_interrupt(0);
            return ERROR;
        }
        if(get_local_ip("wlan0")){
            return OK;
        }
        sleep(1);
        i++;
    }
    return ERROR;
}

