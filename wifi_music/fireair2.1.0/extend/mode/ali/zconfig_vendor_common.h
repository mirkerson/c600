
#include"zconfig_lib.h"


enum {
	ZCONFIG_SCANNING,
	ZCONFIG_CHN_LOCKED,
	ZCONFIG_DONE
};

static u8 g_channel = 6;
static u8 final_channel;
static u8 zconfig_state = ZCONFIG_SCANNING;


//TODO: get active wifi channel list before enter monitor mode
#define MAX_CHANNEL_NUM			(2 * 13 + 5)	/* +5 for safety gap */
static u8 g_channel_list[MAX_CHANNEL_NUM] = {
	6,
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
	/* add scanning channel in the end */
};

zc_callback g_callback;

#ifndef _PLATFORM_LINUX_
u8 g_tpsk[TPSK_LEN + 1];      /* example: 16UQNY5bOiKG4qtjbCMnTeQXziJ9E6yq9zLMZzbnnrY= */
#else
u8 g_tpsk[TPSK_LEN + 1] = product_tpsk;
#endif

/*
 * sniffer result/storage
 * use global variable/buffer to keep it usable after zconfig_destroy
 */
u8 result_ssid[ZC_MAX_SSID_LEN], *res_ssid = NULL;
u8 result_passwd[ZC_MAX_PASSWD_LEN], *res_passwd = NULL;
u8 result_bssid[ETH_ALEN], *res_bssid = NULL;/* mac addr */
u8 result_auth = ZC_AUTH_TYPE_INVALID;
u8 result_encry = ZC_ENC_TYPE_INVALID;
u8 result_channel = 0;


u32 vendor_get_time(void);
void *vendor_malloc(int size);
void vendor_free(void *ptr);
void vendor_msleep(int ms);
char *vendor_get_tpsk(void);
void vendor_printf(int log_level, const char* log_tag, const char* file,
		const char* fun, int line, const char* fmt, ...);

int vendor_monitor_setup(void);
void vendor_monitor_exit(void);
int vendor_channel_switch(u8 channel);

int zconfig_broadcast_notification(char *msg, int msg_num);

char *vendor_get_tpsk(void)
{

	 printf("-----get tpsk-----\n");
    return (char *)g_tpsk;
}

void vendor_channel_locked(u8 channel)
{
	final_channel = channel;
	printf("channel lock @ %d\r\n", channel);

	if (zconfig_state != ZCONFIG_DONE)
		zconfig_state = ZCONFIG_CHN_LOCKED;
}

void vendor_got_ssid_passwd(u8 *ssid, u8 *passwd, u8 *bssid, u8 auth, u8 encry, u8 channel)
{
	if (bssid) {
		printf("xxw ssid: %s, passwd:%s, bssid:%02x%02x%02x%02x%02x%02x, %s, %s, %d\r\n",
				ssid, passwd,
				bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5],
				zconfig_auth_str(auth), zconfig_encry_str(encry), channel);
	} else {
		printf("xxwlyh ssid: %s, passwd:%s, bssid:--, %s, %s, %d\r\n",
				ssid, passwd,
				zconfig_auth_str(auth), zconfig_encry_str(encry), channel);
	}

	memcpy(result_ssid, ssid, ZC_MAX_SSID_LEN);
	res_ssid = &result_ssid[0];

	memcpy(result_passwd, passwd, ZC_MAX_PASSWD_LEN);
	res_passwd = &result_passwd[0];

	if (bssid) {
		memcpy(result_bssid, bssid, ETH_ALEN);
		res_bssid = &result_bssid[0];
	}
	result_auth = auth;
	result_encry = encry;
	result_channel = channel;

	zconfig_state = ZCONFIG_DONE;

#ifdef _PLATFORM_MICO_
	extern void AlinkNotify_AlinkCompleteHandler(network_InitTypeDef_st *nwkpara, mico_Context_t * const inContext);
	extern mico_Context_t *getGlobalContext();

	network_InitTypeDef_st kpara = { 0 };
	strncpy(kpara.wifi_ssid, (const char *)ssid, ZC_MAX_SSID_LEN);
	strncpy(kpara.wifi_key, (const char *)passwd, ZC_MAX_PASSWD_LEN);

	AlinkNotify_AlinkCompleteHandler(&kpara,  getGlobalContext());
#endif
	/*
	 * g_callback(ssid, passwd...) called by vendor_main_thread_func()
	 * no matter zconfig success or timeout
	 */
}

struct zconfig_constructor zconfig = {
	// get system time, in ms.
	&vendor_get_time,
	//malloc & free
	&vendor_malloc,
	&vendor_free,
	//tpsk, released by alibaba
	&vendor_get_tpsk,

	&vendor_printf,

	//callback
	&vendor_channel_locked,
	&vendor_got_ssid_passwd
};

u8 zconfig_next_channel(void)
{
	static int cur = (int)-1;

	while (1) {
		cur++;
		if (cur >= MAX_CHANNEL_NUM)
			cur = 0;   //rollback to start
		if (g_channel_list[cur]) //valid channel
			break;
	}

	return g_channel_list[cur];
}

/* channel scanning/re-scanning control */
void vendor_main_thread_func(void)
{
	unsigned int start_time = vendor_get_time();
	unsigned int security = 0;

	printf("vendor main thread running!\r\n");
rescanning:
	//start scaning channel
	while (zconfig_state == ZCONFIG_SCANNING) {
		//scanning timeout check
		if (vendor_get_time() - start_time > T_ALL_CHANNEL_SCAN) {
			//timeout, setup softAP
			printf("scanning timeout, from %u to %u ms\r\n",
					start_time, vendor_get_time());
			goto timeout_scan;
		}

		g_channel = zconfig_next_channel();
		vendor_channel_switch(g_channel);
		printf("switch to channel %d\r\n", g_channel);

		vendor_msleep(T_ONE_CHANNEL_SCAN);
	}

	//channel hit
	printf("[channel scanning] %u ms\r\n", vendor_get_time() - start_time);
	printf("final channel %d\r\n", final_channel);
	vendor_channel_switch(final_channel);

	while (zconfig_state != ZCONFIG_DONE) {
		vendor_msleep(100);
		if (vendor_get_time() - start_time > T_MONITOR_MODE) {
			printf("recving timeout, from %u to %u ms\r\n",
					start_time, vendor_get_time());
			goto timeout_monitor;
		}
		if (zconfig_state == ZCONFIG_SCANNING) {
			zconfig_printf("channel rescanning...\n");
			goto rescanning;
		}
	}

	printf("[channel recving] %d ms\r\n", vendor_get_time() - start_time);

	goto out;

timeout_scan:
timeout_monitor:
#ifndef AUTO_SWITCH_MODE	
	//goto restart_monitor;
#else	//auto switch to softap
	//ret = vendor_softap_setup();
	//bug_on(ret);
#endif

out:
	/*
	 * exit monitor mode before zconfig_destroy() beacause
	 * it will release mem reource, otherwise sys will crash
	 *
	 * Note: hiflying will reboot after calling this func, so
	 *	g_callback was called in vendor_monitor_exit()
	 */

	vendor_monitor_exit();

	zconfig_destroy();

#ifdef	_PLATFORM_WICED_
	security_trans_zconfig_to_vendor(result_auth, result_encry, &security);
#endif
	/*
	 * g_callback(ssid, passwd...) called by vendor_main_thread_func()
	 * no matter zconfig success or timeout
	 */
	(*g_callback)((char *)res_ssid, (char *)res_passwd, (char *)res_bssid,
			(unsigned int )security, (char)result_channel);

	return;
}



#define UDP_TX_PORT			(65123)
#define UDP_RX_PORT			(65126)
#define bug_on(condition)		while ((condition)) {} ;

static char *zconfig_msg;
#define zconfig_msg_len     (1024)
int zconfig_create_msg(char *model, char *mac, char *sn)
{
	zconfig_msg = vendor_malloc(zconfig_msg_len);
	bug_on(!zconfig_msg);

	memset(zconfig_msg, 0, zconfig_msg_len);

	if (mac && mac[0] != '\0')
		snprintf(zconfig_msg, zconfig_msg_len,
			"{\"model\":\"%s\",\"mac\":\"%s\"}", model, mac);
	else if (sn && sn[0] != '\0')
		snprintf(zconfig_msg, zconfig_msg_len,
			"{\"model\":\"%s\",\"sn\":\"%s\"}", model, sn);
	else {
		zconfig_printf("error, mac/sn not set!\r\n");
		bug_on(1);
	}
	return 0;
}

void zconfig_destroy_msg(void)
{
	if (zconfig_msg) {
		vendor_free(zconfig_msg);
		zconfig_msg = NULL;
	}
}

/*
 * send broadcast msg to APP to stop the zconfig process.
 * this func will block 10~20s.
 * IMPORTANT:
 *      Calling this func as soon as wifi connected(dhcp ready), but
 *      if alink need to do unbind/factory_reset, calling this func after
 *      alink_wait_connect(NULL, 10), 10 means timeout 10s.
 */
void zconfig_notify_app(char *model, char *mac, char *sn)
{
	printf("model:%s, mac:%s, sn:%s\r\n", model, mac, sn);

	if (!model || (!mac && !sn)) {
		printf("model, mac, sn is empty!\r\n");
		bug_on(1);
	}

	zconfig_create_msg(model, mac, sn);
	printf("zconfig_msg: %s\r\n", zconfig_msg);

	zconfig_broadcast_notification(zconfig_msg, 50);

	zconfig_destroy_msg();
}


