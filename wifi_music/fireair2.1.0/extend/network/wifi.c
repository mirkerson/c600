
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
#include <stdbool.h>

#include "wifi.h"
#include "hw.h"
#include "swift_link.h"

static int network_id = 0;
factory_func_callback *ffc_net;

void net_set_external_callback(void *ffc)    
{
    ffc_net = (factory_func_callback *)ffc;
}

int doSysCommand(const char *cmd)
{
    int status;
    if(NULL == cmd)
    {
        return -1;
    }

    status = system(cmd);
    if(status < 0)
    {
        printf("[system]: %s error: %s\n", cmd, strerror(errno));
        return -1;
    }

    //printf("[system]: %s ret: %d\n", cmd, status);

    return (status == 0 ? OK : ERROR);
}


int doPopenCmd(const char *cmd, char *out, int len)
{
    FILE *pstream;
    char buf[1024] = {0};
 
    sprintf(buf,"%s > /tmp/cmdlog.txt",cmd);   
    system(buf);

    LOG_PRT("cmd=%s", buf);
        
    pstream =fopen("/tmp/cmdlog.txt","r");
    if(!pstream)
    {
       printf("open pstream fail!!!\n");
       return ERROR;
    }
    fgets(out,len, pstream);

    LOG_PRT("out=%s", out);
    
    fclose(pstream);

    if(strcmp(out,""))    
        return OK;
    else
        return ERROR;
 
}

static void string_to_toupper(char *src, char *dst)
{  
    while (*src)
    {  
		if(*src >= 'a' && *src <= 'z'){
			*src = *src - ('a'-'A');
		}		
		*dst++ = *src++;	
    }  
}  

static int read_file_node(const char *dev_node,const char *buffer,unsigned len)
{
	int fd;
	int ret ;

	fd = open(dev_node, O_RDONLY);
	if (fd < 0) {
		printf("[r]open %s fail:(%s)\n",dev_node,strerror(errno));
		return ERROR;
	}

	ret = read(fd, (void*)buffer, len);

	if (ret < 0) {
		printf("[r]read %s fail:(%s)\n", dev_node, strerror(errno));
		ret = ERROR;
	}

	if (close(fd) < 0) {
		printf("[r]close %s fail:(%s)\n", dev_node, strerror(errno));
	}

	return OK;
}

char* get_local_ip(char *e_name)
{
        int inet_sock;
        struct ifreq ifr;
        inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

        strcpy(ifr.ifr_name, e_name);
        //SIOCGIFADDR标志代表获取接口地址
        if (ioctl(inet_sock, SIOCGIFADDR, &ifr) <  0)
            return NULL;
        
        printf("%s\n", inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
        return inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr);
}

int getHwaddr(const char *if_name, char *mac)
{
	char dev_node[64] = {0};
	char value[32] = {0};

	const char *sys_fs_dir = "/sys/class/net/";

	snprintf(dev_node, sizeof(dev_node), "%s%s/address", sys_fs_dir, if_name);

	if (read_file_node(dev_node,value,sizeof(value)) != OK) 
		return ERROR;

    value[17] ='\0';

	string_to_toupper(value, mac);	

	return OK;
}

int getWifiMode(void)
{
	FILE *fp;
	char buf[32]={0};
	int mode = 0;
	
	if((fp=fopen("/tmp/conf/config.dat","r"))!=NULL)
	{
		if(fgets(buf,sizeof(buf),fp)!=NULL)
		{
			mode = (buf[0] == '1' ? STATION : HOST_AP);
		}
		fclose(fp);
	}

	return mode;
}

void getAPConfig(char *ap_name,char *ap_pwd ,int always_from_file)
{
	FILE *fp;
	int i;
	char buf[128]={0};
	
	if((fp=fopen("/tmp/conf/config.dat","r"))!=NULL)
	{
		for(i=1;i<4;i++)
		{
			if(fgets(buf,sizeof(buf),fp)!=NULL)
			{
				//ssid
				if(i==2)	//line 2
				{
					sscanf(buf,"%127s",ap_name);
				}

				//password
				if(i==3)	//line 3
				{
					sscanf(buf,"%127s",ap_pwd);
				}
				
				memset(buf,0x0,sizeof(buf));
			}
		}
		fclose(fp);
	}

}

void getSTAConfig(char *sta_type,char *sta_name,char *sta_pwd)
{
	FILE *fp;
	int i;
	char buf[128]={0};
	
	if((fp=fopen("/tmp/conf/config.dat","r"))!=NULL)
	{
		for(i=1;i<7;i++)
		{
			if(fgets(buf,sizeof(buf),fp)!=NULL)
			{
				//type
				if(i==4)	//line 4
				{
					sscanf(buf,"%31s",sta_type);
				}

				//ssid
				if(i==5)	//line 5
				{
					sscanf(buf,"%127s",sta_name);
				}

				//password
				if(i==6)	//line 6
				{
					sscanf(buf,"%127s",sta_pwd);
				}
				
				memset(buf,0x0,sizeof(buf));
			}
		}
		fclose(fp);
	}

}

void startAp(char *ap_name,char *ap_pwd, int flag)
{
	char cmd[128]={0};

	sprintf(cmd,"/usr/sbin/softap.sh 0 \"%s\" \"%s\"",ap_name,ap_pwd);

	printf("[startAp] -> %s\n",ap_name);
    doSysCommand(cmd);  

    if(flag > 0)
    {               
        doSysCommand("/usr/sbin/save.sh 3");
    }

}


void stopAp()
{
	char cmd[128]={0};

	//doSysCommand("ifconfig wlan0 down");

	sprintf(cmd,"killall -q hostapd udhcpd");

	printf("[stopAp] -> %s\n",cmd);

	doSysCommand(cmd);

}

int fireair_gen_wpafile(int encryptType, char *ssid, char *pwd)
{
    FILE *fp;
    if((fp=fopen("/tmp/conf/wpa_supplicant.conf", "w+"))==NULL){
        printf("open file failed %s\n", strerror(errno));
        exit(1);
    }
    char commset[]={"ctrl_interface=/var/run/wpa_supplicant\nctrl_interface_group=wheel\nupdate_config=1\neapol_version=1\nfast_reauth=1\n"};
    char OPENstr[]={"ap_scan=1\nnetwork={\n\tssid=\"%s\"\n\tkey_mgmt=NONE\n\tscan_ssid=1\n}\n"};
    char WPAstr[]={"ap_scan=1\nnetwork={\n\tssid=\"%s\"\n\tscan_ssid=1\n\tpsk=\"%s\"\n\tkey_mgmt=WPA-PSK\n}\n"};
    char WEPstr[]={"ap_scan=1\nnetwork={\n\tssid=\"%s\"\n\tkey_mgmt=NONE\n\twep_key0=\"%s\"\n\twep_tx_keyidx=0\n\tscan_ssid=1\n}\n"};
    char CmdStr[2048] = {0};
    sprintf(CmdStr, "%s", commset);
    fprintf(fp,"%s", CmdStr);
    switch (encryptType) {
        case SECURITY_NONE:
            sprintf(CmdStr, OPENstr,ssid);
            break;
        case SECURITY_WPA :
            sprintf(CmdStr, WPAstr, ssid, pwd);
            break;
        case SECURITY_WEP :
            sprintf(CmdStr, WEPstr, ssid, pwd);
            break;
        default :
            return -EINVAL;
            break;
    }
    //printf("%s\n",CmdStr);
    fprintf(fp,"%s", CmdStr);
    fclose(fp);
    return 0;
}
int get_ap_encrypt_type(char *ssid, key_mgmt_t *encryptType)
{
    FILE *fp = NULL;
    char buf[256] = "";
    char *tmp = NULL;
    int len = 0;
    bool first_decode = false;
    int ret = -1;
    char tmpe_ssid[32 + 1] = "";
    key_mgmt_t mgmt = SECURITY_INV;

    
    char cmd[64] = "";
    doSysCommand("ifconfig wlan0 up");
    sprintf(cmd, "iw dev %s scan > %s", "wlan0", "/tmp/scan.txt");
    doSysCommand(cmd);
    
    if((fp=fopen("/tmp/scan.txt", "r")) == NULL){
        printf("open file failed %s\n", strerror(errno));
        return -1;
    }

    while(fgetc(fp) != EOF)
    {
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf), fp);
        tmp = strstr(buf, "wlan0");
        if(tmp){

            if(first_decode){
                if(!memcmp(ssid, tmpe_ssid, strlen(ssid))){
                    if(mgmt == SECURITY_INV){
                        *encryptType = SECURITY_WEP;
                    }else{
                        *encryptType = mgmt;
                    }
                    ret = 0;
                    break;
                }
                memset(tmpe_ssid, '\0', 33);
            }
            first_decode = true;
        }
        tmp = strstr(buf, "capability: ESS");
        if(tmp){
            if(NULL == strstr(tmp, "Privacy"))
                mgmt = SECURITY_NONE;
        }
        tmp = strstr(buf, "SSID: ");
        if(tmp){
            tmp += 6;
            if((len =strlen(tmp)) > 33){
                len = 33;
            }
            tmp[len-1] ='\0';
            memcpy(tmpe_ssid, tmp, len);
        }
        if(strstr(buf, "RSN:")){
            mgmt = SECURITY_WPA;
        }
        if(strstr(buf, "WPA:")){
            mgmt = SECURITY_WPA;
        }
    }
    if(NULL != fp){
        fclose(fp);
        fp = NULL;
    }
    memset(cmd, '\0', sizeof(cmd));
    sprintf(cmd, "rm -rf %s", "tmp/scan.txt");
    doSysCommand(cmd);
    return ret;
}

int staConnect(char *encryptType, char *ssid,char *pwd)
{
    char cmd[256] = "";
    key_mgmt_t enc_type = SECURITY_INV;
    doSysCommand("killall -q udhcpc wpa_supplicant");
    ffc_net->driver.set_status(LED_WIFI_CONNECTING);

    if(strstr(encryptType, "WPA"))
	{
        enc_type = SECURITY_WPA;
    }
	else if(strstr(encryptType, "WEP"))
	{
        enc_type = SECURITY_WEP;
    }
	else if(strstr(encryptType, "NONE"))
	{
        enc_type = SECURITY_NONE;
    }
	else if(strstr(encryptType, "INV"))
	{
        while(!get_ap_encrypt_type(ssid, &enc_type))
            usleep(50*1000);
	}

    sprintf(cmd,"/usr/sbin/save.sh 1 '%s' '%s' '%s'", key_mgmt_str[enc_type], ssid, pwd);
    doSysCommand(cmd);
    fireair_gen_wpafile(enc_type, ssid, pwd);
    doSysCommand("ifconfig wlan0 0.0.0.0");
    doSysCommand("wpa_supplicant -iwlan0 -Dnl80211 -c /tmp/conf/wpa_supplicant.conf -B");
    //DHCP
    doSysCommand("udhcpc -iwlan0 &");
    //  doSysCommand("mdnsd");

	return OK;

}

int staDisConnect()
{
	char buf[64]="";
	char cmd[128]="";

	printf("[staDisConnect]\n");

	sprintf(cmd,"wpa_cli -iwlan0 disable_network %d",network_id);
	if( OK != doPopenCmd(cmd,buf,sizeof(buf)) )
	{
		if(strncmp("OK",buf,2))
		{
			printf("[staConn] %s -> %s\n",cmd,buf);
			return ERROR;
		}
	}

	doSysCommand("killall -q udhcpc");

	return OK;
}

int getstaStatus(char *retStatus,int len)
{
	char cmd[128]="";

	sprintf(cmd,"wpa_cli -i wlan0 status | grep 'wpa_state' | cut -d '=' -f 2");
	if( OK != doPopenCmd(cmd,retStatus,len) )
	{
		return ERROR;
	}

	return OK;
}

int wifi_init(void)
{
    char ssid[128],pwd[128],encryptType[32];
        
	if(HOST_AP == getWifiMode())
	{
		getAPConfig(ssid,pwd,0);

        ffc_net->driver.set_status(LED_WIFI_DISCONNECT);

		startAp(ssid,pwd, 1);

        ffc_net->player.tone_play("/home/ad/weilianwang.mp3", 0);

        return HOST_AP;

	}
	else if(STATION == getWifiMode())
	{    
        
		getSTAConfig(encryptType,ssid,pwd);

		staConnect(encryptType,ssid,pwd);

        ffc_net->player.tone_play("/home/ad/zailianwang.mp3", 0);


        int cnt = 0;
        char cmd[128] = {0};
        char buf[128] = "";
        
    	while(cnt < 120)
    	{
            sprintf(cmd,"ifconfig wlan0 | grep \"inet addr\"");
            
            if( (NULL != cmd) && (OK == doPopenCmd(cmd,buf,sizeof(buf))) )
            {
                if(strcmp(buf,""))
                {
                    ffc_net->driver.set_status(LED_WIFI_CONNECTED);
                
                    system("/home/tinyplay /home/ad/lianwangok.wav");
                    system("/home/tinyplay /home/ad/lianwangok.wav -D 1");
                                   
                    //check_to_play_sd();  
                                    
                    return STATION;
                }
            } 

            usleep(500000);
            cnt ++;
            
    	}

        printf("connect network fail,please re-config.\n");

        ffc_net->player.tone_play("/home/ad/jianchawangluo.mp3", 0);

        return INVALID_NET;

    
	}
    else
    {
        printf("invalid wifi mode!\n");

        return INVALID_NET;
    }
    
}

int check_network_status(void)
{
    int ret = ERROR;
    char buf[128];
    
    if(STATION == getWifiMode())
    {
        if( OK == getstaStatus(buf,sizeof(buf)) && !strncmp(buf,"COMPLETED",9) )
            ret = OK;
    }

    return ret;
}


