
#ifdef __ALI_MUSIC__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset()

#include <netinet/ip.h>       // IP_MAXPACKET (65535)
#include <sys/time.h>
#include <net/ethernet.h>     // ETH_P_ALL
#include <net/if.h>	      // struct ifreq
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <net/if.h>

#include <pthread.h>
#include "zconfig_lib.h"
#include "zconfig_export.h"
#include "ali_music.h"

#define _PLATFORM_LINUX_

//TODO: update these product info
#define product_model		"ALINKTEST_ENTERTAINMENT_ATALK_QUANZHI"
#define product_tpsk		"PLnBaCPHF7icf65a5nJmcL2GZC+w3vwCnH36k8O91og="	//44bytes len
//#define device_mac		"00:01:02:03:04:05"
#define device_sn		"0001020304050607"

char *vendor_get_model(void) { return product_model; }
char *vendor_get_sn(void) { return device_sn; }

#define T_MONITOR_MODE		(60)//second//60
#define T_ONE_CHANNEL_SCAN 	(400 * 1000)//us, timeout for one channel scanning
#define T_ALL_CHANNEL_SCAN 	(T_MONITOR_MODE)
#define T_SELECT_TIMEOUT	(T_ONE_CHANNEL_SCAN)//us
#define T_CHANNEL_LOCK_TIMEOUT	(4 * 1000)//ms//4

#define zconfig_printf		printf
#include "zconfig_vendor_common.h"


static char enable_log = 0;
#define log(format, ...)		\
	do {\
		if (enable_log)			\
		printf(format, ##__VA_ARGS__);	\
	} while (0)

#define err(format, ...)	printf(format, ##__VA_ARGS__)
#define info(format, ...)	printf(format, ##__VA_ARGS__)

#define IFNAME			"wlan0"	//criss


static char thread_monitor_running = 1;
static char *ether_frame;

#define MAC_ADRESS_FILE_NAME   "/tmp/conf/mac-address.conf"
void vendor_get_mac(char *mac) 
{   
	int ret = access(MAC_ADRESS_FILE_NAME, 0);
	if (ret < 0)
	    return;
    
	FILE *f = fopen(MAC_ADRESS_FILE_NAME, "r");
	fscanf(f, "%18s", mac);
	fclose(f);

}

void *vendor_malloc(int size)
{
	return malloc(size);
}

void vendor_free(void *ptr)
{
	free(ptr);
}

unsigned int vendor_get_time(void)
{
	struct timeval tv = { 0 };
	unsigned int time_ms;

	gettimeofday(&tv, NULL);

	time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	return time_ms;
}

void vendor_msleep(int ms)
{
	usleep(ms * 1000);
}

void vendor_printf(int log_level, const char* log_tag, const char* file,
		const char* fun, int line, const char* fmt, ...)
{
	va_list args;
	char log_buf[256];

	va_start(args, fmt);

	vsnprintf(&log_buf[0], sizeof(log_buf), fmt, args);

	va_end(args);

	printf("%s", log_buf);
}

//TODO:
int vendor_channel_switch(u8 channel)
{
	char cmd[64];
	sprintf(cmd, "iw dev %s set channel %d", IFNAME, channel);
	system(cmd);
}


void wifi_disable(const char* ifName){
	char cmd[64];
	sprintf(cmd, "ifconfig %s down", ifName);
	system(cmd);

}

void wifi_enable(const char* ifName){
	char cmd[64];
	sprintf(cmd, "ifconfig %s up", ifName);
	system(cmd);
	sleep(1);
}

void clear_wpas(void)
{
    system("wpa_cli -iwlan0 disconnect");

    //TODO and also clear the conf file
	system("killall wpa_supplicant 2 >/dev/null");
    system("killall udhcpc  2 >/dev/null");
    
    system("killall hostapd 2 >/dev/null");
	system("killall udhcpd  2 >/dev/null");   
}

//TODO:
int vendor_monitor_setup(void)
{
	char buf[256];

	wifi_disable(IFNAME);

	//snprintf(buf, sizeof(buf), "iwconfig %s mode monitor", IFNAME);
	sprintf(buf, "iw %s set type monitor", IFNAME);
	system(buf);

	wifi_enable(IFNAME);

	vendor_channel_switch(6);  //switch to channel 6, default

	return 0;
}

//TODO:
void vendor_monitor_exit(void)
{
	char buf[256] = {0};

	wifi_disable(IFNAME);

	sprintf(buf, "iw %s set type station", IFNAME);
	system(buf);

	wifi_enable(IFNAME);
}

//TODO:
void vendor_station_setup(char *ssid, char *passwd)
{
	char buf[1024];

	printf("ssid=%s, passwd=%s \n", ssid, passwd);

    if( (NULL != ssid) && (NULL != passwd) )
    {
        sprintf(buf,"/usr/sbin/save.sh 1 unknown '%s' '%s'",ssid,passwd);
        system(buf);   
    }

    printf("result_encry=%d\n", result_encry);
    if( result_encry == ZC_ENC_TYPE_AES)
	{
		sprintf(buf, "echo -e \"update_config=1\neapol_version=1\nfast_reauth=1\nap_scan=1\nnetwork={\n\tssid=\\\"%s\\\"\n\\tscan_ssid=1\n\tpsk=\\\"%s\\\"\n}\n\" > /tmp/wpa_supplicant.conf", ssid, passwd);
		system(buf);
	}	
	if( result_encry == ZC_ENC_TYPE_WEP)
	{
		sprintf(buf, "echo -e \"update_config=1\neapol_version=1\nfast_reauth=1\nap_scan=1\nnetwork={\n\tssid=\"%s\"\n\tkey_mgmt=NONE\n\twep_key0=\"%s\"\n\twep_tx_keyidx=0\n\tscan_ssid=1\n}\n\" > /tmp/wpa_supplicant.conf", ssid, passwd);
		system(buf);
	}
	if( result_encry == ZC_ENC_TYPE_NONE)
	{
		sprintf(buf, "echo -e \"update_config=1\neapol_version=1\nfast_reauth=1\nap_scan=1\nnetwork={\n\tssid=\"%s\"\n\tkey_mgmt=NONE\n\tscan_ssid=1\n}\n\" > /tmp/wpa_supplicant.conf", ssid);
		system(buf);
	}


	sprintf(buf, "wpa_supplicant -i %s -c /tmp/wpa_supplicant.conf -Dnl80211 &", IFNAME);
	system(buf);
    
	sleep(2);

	sprintf(buf, "udhcpc -i%s &", IFNAME);
	system(buf);

}


int vendor_auth_encty(char *auth, char *encty, char *ssid, char *bssid, int channel)
{
	if(!strcmp(auth,"WPA2-PSK"))
	{
		if(!strcmp(encty,"CCMP"))
		{
			printf(".....WPA2-PSK-CCMP setting.....\n");
			zconfig_set_apinfo(ssid, bssid, channel, ZC_AUTH_TYPE_WPA2PSK, ZC_ENC_TYPE_AES);
			g_channel_list[13 + channel] = channel;
		}
		if(!strcmp(encty,"TKIP"))
		{
			printf(".....WPA2-PSK-TKIP setting.....\n");
			zconfig_set_apinfo(ssid, bssid, channel, ZC_AUTH_TYPE_WPA2PSK, ZC_ENC_TYPE_TKIP);
			g_channel_list[13 + channel] = channel;
		}
		if(!strcmp(encty,"CCMP TKIP")||!strcmp(encty,"TKIP CCMP"))
		{
			printf(".....WPA2-PSK-CCMP-TKIP setting.....\n");
			zconfig_set_apinfo(ssid, bssid, channel, ZC_AUTH_TYPE_WPA2PSK, ZC_ENC_TYPE_TKIPAES);
			g_channel_list[13 + channel] = channel;
		}
	}

	if(!strcmp(auth,"WPA-PSK"))
	{
		if(!strcmp(encty,"CCMP"))
		{
			printf(".....WPA-PSK-CCMP setting.....\n");
			zconfig_set_apinfo(ssid, bssid, channel, ZC_AUTH_TYPE_WPAPSK, ZC_ENC_TYPE_AES);
			g_channel_list[13 + channel] = channel;
		}
		if(!strcmp(encty,"TKIP"))
		{
			printf(".....WPA-PSK-TKIP setting.....\n");
			zconfig_set_apinfo(ssid, bssid, channel, ZC_AUTH_TYPE_WPAPSK, ZC_ENC_TYPE_TKIP);
			g_channel_list[13 + channel] = channel;
		}
		if(!strcmp(encty,"TKIP CCMP")||!strcmp(encty,"CCMP TKIP"))
		{
			printf(".....WPA-PSK-CCMP-TKIP setting.....\n");
			zconfig_set_apinfo(ssid, bssid, channel, ZC_AUTH_TYPE_WPAPSK, ZC_ENC_TYPE_TKIPAES);
			g_channel_list[13 + channel] = channel;
		}
	}
	if(!strcmp(auth,"WPA2-PSK-WPA-PSK"))
	{
		if(!strcmp(encty,"CCMP-CCMP"))
		{
			printf(".....WPA2-PSK-WPA-PSK-CCMP setting.....\n");
			zconfig_set_apinfo(ssid, bssid, channel, ZC_AUTH_TYPE_WPAPSKWPA2PSK, ZC_ENC_TYPE_AES);
			g_channel_list[13 + channel] = channel;
		}
		if(!strcmp(encty,"TKIP-TKIP"))
		{
			printf(".....WPA2-PSK-WPA-PSK-TKIP setting.....\n");
			zconfig_set_apinfo(ssid, bssid, channel, ZC_AUTH_TYPE_WPAPSKWPA2PSK, ZC_ENC_TYPE_TKIP);
			g_channel_list[13 + channel] = channel;
		}
		if(!strcmp(encty,"TKIP-CCMP")||!strcmp(encty,"CCMP TKIP-CCMP TKIP"))
		{
			printf(".....WPA2-PSK-WPA-PSK-CCMP-TKIP setting.....\n");
			zconfig_set_apinfo(ssid, bssid, channel,  ZC_AUTH_TYPE_WPAPSKWPA2PSK, ZC_ENC_TYPE_TKIPAES);
			g_channel_list[13 + channel] = channel;
		}
	}

}

int vendor_prepare_channel_list(void)
{
	int scan_num = 0;
	FILE *fp = NULL;
	char buf[512]  = {0};
	char rst[32]   = {0};
	char bssid[32] = {0};
	char ssid[256]  = {0};
	char encty[32] = {0};
	char auth[32]  = {0};

	char *tmp = NULL;
	int fops = 0;
	int len = 0;
	int enable = 1;
	int channel = 0;
	int signal = 0;


	system("iw dev wlan0 scan >/tmp/scan.txt");
	fp = fopen("/tmp/scan.txt","r");
	if(!fp)
	{
		printf("open /tmp/scan.txt fail!!!\n");
		return -1;
	}
	while((fops = fgetc(fp)) != EOF)
	{
		memset(buf, 0, sizeof(buf));
		fgets(buf,sizeof(buf),fp);
		tmp = strstr(buf,"wlan0");
		if(tmp)
		{
wlan0:
			enable = 1;
			tmp -= 22;
			tmp[17] = '\0';
			memset(bssid,0,sizeof(bssid));
			strcpy(bssid,tmp);
			printf("BSSID=%s\n",bssid);

			while(enable &&(fgetc(fp)) != EOF)
			{
				memset(buf, 0, sizeof(buf));
				fgets(buf,sizeof(buf),fp);

				tmp = strstr(buf,"wlan0");
				if(tmp)
				{
					enable = 0;
					printf("--------enable:%d--------\n",enable);
	   		        vendor_auth_encty(auth,encty,ssid,bssid,channel);
					goto wlan0;
				}
				tmp = strstr(buf,"signal: ");
				if(tmp)
				{
					tmp += 8;
					len =strlen(tmp);
					tmp[len-1] ='\0';
					signal = atoi(tmp);
					printf(" |signal:%d\n",signal);
				}

				tmp = strstr(buf,"SSID: ");
				if(tmp)
				{
					tmp += 6;
					len =strlen(tmp);
					tmp[len-1] ='\0';

					memset(ssid,0,sizeof(ssid));
					strcpy(ssid,tmp);
					printf(" |SSID:%s\n",ssid);
				}

				tmp = strstr(buf,"set: channel ");
				if(tmp)
				{
					tmp += 13;
					len = strlen(tmp);
					tmp[len-1] = '\0';
					channel = atoi(tmp);
					printf(" |channel:%d\n",channel);
				}

				tmp = strstr(buf,"RSN:");
				if(tmp)
				{
					fgets(buf,sizeof(buf),fp);
					memset(buf,0,sizeof(buf));
					fgets(buf,sizeof(buf),fp);
					tmp = strstr(buf,"ciphers: ");
					if(tmp)
					{
						tmp += 9;
						len = strlen(tmp);
						tmp[len-1] = '\0';
						printf(" |chipers:%s\n",tmp);
						memset(rst, 0, sizeof(rst));
						strcpy(rst,tmp);
						memset(buf,0,sizeof(buf));
						fgets(buf,sizeof(buf),fp);
						tmp = strstr(buf,"suites: ");
						if(tmp)
						{
							tmp += 8;
							len = strlen(tmp);
							tmp[len-1] = '\0';
							memset(encty,0,sizeof(encty));
							memset(auth,0,sizeof(auth));
							strcpy(encty,rst);
							sprintf(auth,"WPA2-%s",tmp);
							printf("	|AUTH:%s\n	|ENCTY:%s\n",auth,encty);
						}

					}
				}
				tmp = strstr(buf,"WPA:");
				if(tmp)
				{
					tmp += 5;
					len = strlen(tmp);
					tmp[len-1] = '\0';
					fgets(buf,sizeof(buf),fp);
					memset(buf,0,sizeof(buf));
					fgets(buf,sizeof(buf),fp);
					tmp = strstr(buf,"ciphers: ");
					if(tmp)
					{
						tmp += 9;
						len = strlen(tmp);
						tmp[len-1] = '\0';
						printf(" |chinpers:%s\n",tmp);
						memset(buf,0,sizeof(buf));
						fgets(buf,sizeof(buf),fp);
						tmp = strstr(buf,"suites: ");
						if(tmp)
						{
							tmp += 8;
							len = strlen(tmp);
							tmp[len-1] = '\0';

							len = strlen(encty);
							if(len)
							{
							   sprintf(encty+len,"-%s",rst);
							}
							else
							{
							   strcpy(encty,rst);
							}

							len =  strlen(auth);
							if(len)
							   sprintf(auth+len,"-WPA-%s",tmp);
							else
								sprintf(auth,"WPA-%s",tmp);
							printf("	|AUTH:%s\n	|ENCTY:%s\n",auth,encty);
						}

					}
				}
			}
		}
	}
    vendor_auth_encty(auth,encty,ssid,bssid,channel);

	fclose(fp);
	return 0;
}



/* return time used in us */
int time_calc_usec(struct timeval *start)
{
	struct timeval now;
	int time_us;

	gettimeofday(&now, NULL);

	time_us = (now.tv_sec - start->tv_sec) * 1000000
		+ (now.tv_usec - start->tv_usec);

	return time_us;
}

int open_socket(void)
{
	int fd;
#if 0
	if (getuid() != 0)
		err("root privilege needed!\n");
#endif
	//create a raw socket that shall sniff
	fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	assert(fd >= 0);

	struct ifreq ifr;
	int sockopt = 1;

	memset(&ifr, 0, sizeof(ifr));

	/* set interface to promiscuous mode */
	strncpy(ifr.ifr_name, IFNAME, sizeof(ifr.ifr_name));
	if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
		perror("ioctl(SIOCGIFFLAGS)");
		goto exit;
	}
	ifr.ifr_flags |= IFF_PROMISC;
	if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
		perror("ioctl(SIOCSIFFLAGS)");
		goto exit;
	}

	/* allow the socket to be reused - incase connection is closed prematurely */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) < 0) {
		perror("setsockopt(SO_REUSEADDR)");
		goto exit;
	}

	/* bind to device */
	struct sockaddr_ll ll;

	memset(&ll, 0, sizeof(ll));
	ll.sll_family   = PF_PACKET;
	ll.sll_protocol = htons(ETH_P_ALL);
	ll.sll_ifindex  = if_nametoindex(IFNAME);
	if (bind(fd, (struct sockaddr *)&ll, sizeof(ll)) < 0) {
		perror("bind[PF_PACKET] failed");
		goto exit;
	}

	return fd;
exit:
	close(fd);
	exit(EXIT_FAILURE);
}

/* recv raw data */
int zconfig_recv_raw_package(int fd)
{
	static unsigned int lock_start;

	int len, ret;
	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	tv.tv_sec = 0;
	tv.tv_usec = T_SELECT_TIMEOUT;

	ret = select(fd + 1, &rfds, NULL, NULL, &tv);
	if (ret <= 0)
		return -1;

	memset(ether_frame, 0, IP_MAXPACKET);
	len = recv(fd, ether_frame, IP_MAXPACKET, 0);
	if (len < 0) {
		perror ("recv() failed:");
		//Something weird happened
		return -2;
	}

	if(len >1000)
	{
	    log("----recv_data len=%d----\n",len);
		//int i;
		//printf("ether_frame[0-9] is:" );
		//for(i=0;i<10;i++)
		//	printf("%x ",ether_frame[i]);
		//printf("\n");


	}

	/* len - 4, droping 80211 crc field */
	//ret = zconfig_recv_callback(ether_frame, len-4 , g_channel);//TODO:
    ret = zconfig_recv_callback(ether_frame, len, g_channel);
	switch (ret)
	{
		case PKG_START_FRAME:
			log("start frame\n");
			break;
		case PKG_DATA_FRAME:
			log("data frame\n");
			break;
		case PKG_GROUP_FRAME:
			log("group frame\n");
			break;
		default:
			break;
	}

	if (zconfig_state == ZCONFIG_CHN_LOCKED) {
		log("ssid locked!!!\n");
		switch (ret) {
			case PKG_START_FRAME:
				log("start frame\n");
			case PKG_DATA_FRAME:
				log("data frame\n");
			case PKG_GROUP_FRAME:
				lock_start = vendor_get_time();
				log("group frame\n");
				break;
			default:
				if (vendor_get_time() - lock_start > T_CHANNEL_LOCK_TIMEOUT)
				{
					/* set to rescanning */
					log("set to rescanning\n");
					zconfig_state = ZCONFIG_SCANNING;
				}
				break;
		}
	}

	return ret;
}

void parse_opt(int argc, char **argv)
{
	int i;

	for (i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "--log") || !strcmp(argv[i], "-l"))
			enable_log = 1;
	}
}

void *vendor_sniffer_thread_func(void *arg)
{
	int sd;
	struct timeval start;
	int state;

	g_channel = zconfig_next_channel();
	vendor_channel_switch(g_channel);
	gettimeofday(&start, NULL);

  	sd = open_socket();

rescanning:
	/* channel scanning */
	while (thread_monitor_running && zconfig_state == ZCONFIG_SCANNING) {
		int time_used = time_calc_usec(&start);
		int delay = 0;
		if (time_used > T_ONE_CHANNEL_SCAN) {
			/* switch to next channel */

			//FIXME: close the socket before channel switch,
			//otherwise it'll bug on some platform
			//close(sd);

			gettimeofday(&start, NULL);
			g_channel = zconfig_next_channel();
			vendor_channel_switch(g_channel);
			log("switch to channel %d\n", g_channel);

			/* FIXME: reopen the socket, to drop the kernel buffer */
			//sd = open_socket();
		}

		state = zconfig_recv_raw_package(sd);
		switch (state) {
			case PKG_START_FRAME:
				log("channel lock @ %d\n", g_channel);
				goto start_capture;
			case PKG_BC_FRAME:
				/* stay awhile */
				if (delay++ < T_CHANNEL_LOCK_TIMEOUT)
					start.tv_usec += T_ONE_CHANNEL_SCAN;
				break;
			default:
				break;
		}

	}
    
	//channel_lock:
	if (!thread_monitor_running)
		goto thread_exit;

start_capture:
	gettimeofday(&start, NULL);/* update timestamp */
	vendor_channel_switch(final_channel);
	printf("channel locked success!!\n");

	while (thread_monitor_running && zconfig_state != ZCONFIG_DONE) {
		/*
		 * keep recving until thread timeout
		 * zconfig_got_ssid_passwd() will set ZCONFIG_DONE
		 */
		state = zconfig_recv_raw_package(sd);
		switch (state) {
			case PKG_START_FRAME:
			case PKG_GROUP_FRAME:
			case PKG_DATA_FRAME:
			case PKG_ALINK_ROUTER:
				/* everything goes well */
				gettimeofday(&start, NULL);/* update timestamp */
				break;
			default:
				break;
		}

		if (zconfig_state == ZCONFIG_SCANNING) {
			info("channel rescanning...\n");
			goto rescanning;
		}
	}

	//gettimeofday(&start, NULL);/* update timestamp */

thread_exit:

	thread_monitor_running = 0;
	close(sd);

	pthread_exit(NULL);
}

static char* ali_get_ip(char *e_name)
{
        int inet_sock;
        struct ifreq ifr;
        inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

        printf("ali_get_ip-----\n");

        strcpy(ifr.ifr_name, e_name);
        //SIOCGIFADDR标志代表获取接口地址
        if (ioctl(inet_sock, SIOCGIFADDR, &ifr) <  0)
            return NULL;
        
        printf("%s\n", inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr));
        
        return inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr);
}

void* ali_notify_app_thread(void *arg)
{

	char mac_address[32] = {0};
	
	vendor_get_mac(mac_address);
   
    /* Calling zconfig_notify_app as soon as wifi connected(dhcp ready), but
     * if alink need to do unbind/factory_reset, calling this func after
     * alink_wait_connect(NULL, 10), 10 means timeout 10s.
     */

    zconfig_notify_app(vendor_get_model(),(char *)mac_address, vendor_get_sn());

}

int ali_notify_app(void)
{
	int ret;
	pthread_t ali_notify = 0;

    ret = pthread_create(&ali_notify, NULL, &ali_notify_app_thread, NULL);
	if(ret < 0)
	{
		printf("pthread_create error %s\n", strerror(errno));
		return -1;
	}

    return 0;
}

int ali_simple_config(void)
{
	int ret, seconds;
    int i=0;
    //char mac[32] = {0};
	//parse_opt(argc, argv);
    pthread_t thread_monitor;

	ether_frame = malloc(IP_MAXPACKET);
	assert(ether_frame);

//	ffc_inst.network.disconnect();
	clear_wpas();
        
	zconfig_init(&zconfig);
    
    vendor_monitor_exit();

	vendor_prepare_channel_list();
    sleep(1);
         
	vendor_monitor_setup();
    
	thread_monitor_running = 1;

	ret = pthread_create(&thread_monitor, NULL, &vendor_sniffer_thread_func, NULL);

	assert(!ret);
   
	seconds = T_MONITOR_MODE;
	while (--seconds && thread_monitor_running)        
        sleep(1);
    
	thread_monitor_running = 0;
	pthread_join(thread_monitor, NULL);

	free(ether_frame);

	zconfig_destroy();

    vendor_monitor_exit();
	if (!seconds) {
		info("monitor mode timeout!\n");
		return 1;
	}

	info("monitor mode: switch to station mode\n");

	ffc_inst.network.connect("unknown", res_ssid, res_passwd);
    
	//vendor_station_setup((char *)res_ssid, (char *)res_passwd);
    printf("conecting.............. \n");
    
	system("rm /tmp/scan.txt");

    while(i < 60)
    {

        if(NULL != ali_get_ip(IFNAME)) 
        {    
            //system("/home/tinyplay /home/ad/lianwangok.wav");
            //system("/home/tinyplay /home/ad/lianwangok.wav -D 1");


            if(0 != ali_notify_app())
                return 1;

            //vendor_get_mac(mac);
	        //zconfig_notify_app(vendor_get_model(), mac, vendor_get_sn());
    
            return 0;  
        }            
        
        sleep(1);
        
        i++;
            
    }

	printf("---exit zconfig----\n");

	return 1;
}

int zconfig_broadcast_notification(char *msg, int msg_num)
{
	struct sockaddr_in sock_in;
	int fd, ret, socklen, i;
	int yes = 1;

	int buf_len = 512;
	char *buf = malloc(buf_len);

	socklen = sizeof(struct sockaddr_in);

	memset(&sock_in, 0, sizeof(sock_in));
	sock_in.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_in.sin_port = htons(UDP_RX_PORT);
	sock_in.sin_family = PF_INET;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	bug_on(fd < 0);

	ret = bind(fd, (struct sockaddr *)&sock_in, sizeof(sock_in));
	bug_on(ret);

	ret = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(int) );
	bug_on(ret);

	/*
	 * -1 = 255.255.255.255 this is a BROADCAST address,
	 * a local broadcast address could also be used.
	 * you can comput the local broadcat using NIC address and its NETMASK
	 */
	sock_in.sin_addr.s_addr = htonl((uint32_t)-1); /* send message to 255.255.255.255 */
	sock_in.sin_port = htons(UDP_TX_PORT); /* port number */
	sock_in.sin_family = PF_INET;
	//send notification
	for (i = 0; i < msg_num; i++) {
		ret = sendto(fd, msg, strlen(msg), 0,
				(struct sockaddr *)&sock_in, socklen);
		if (ret < 0) {
			perror ("sendto() failed:");
			break;
		} else
			info("broadcast: %s\n", msg);

		do {
			struct timeval tv;
			fd_set rfds;

			FD_ZERO(&rfds);
			FD_SET(fd, &rfds);

			tv.tv_sec = 0;
			tv.tv_usec = 1000 * (200 + i * 100); //from 200ms to N * 100ms

			ret = select(fd + 1, &rfds, NULL, NULL, &tv);
			if (ret > 0) {
				ret = recv(fd, buf, buf_len, 0);
				if (ret) {
					buf[ret] = '\0';
					printf("rx: %s\n", buf);
					goto out;
				}
			}
		} while (0);
	}

out:
	close(fd);
	free(buf);

	ret = 0;

	return ret;
}

#endif

