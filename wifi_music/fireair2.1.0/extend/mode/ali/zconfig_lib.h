#ifndef __ZCONFIG_LIB_H
#define __ZCONFIG_LIB_H

#ifndef u8
#define u8				unsigned char
#define u16				unsigned short
#define u32				unsigned int
#endif

#ifndef ETH_ALEN
#define ETH_ALEN			6
#endif

#define	ZC_MAX_SSID_LEN			(32 + 1)/* ssid: 32 octets at most, include the NULL-terminated */
#define ZC_MAX_PASSWD_LEN			(63 + 1)/* 8-63 ascii */

//used by tpsk
#define TPSK_LEN			(44)
#define ALINK_IE_HDR			(7)

enum _LOGLEVEL_ {
	LOGLEVEL_NONE,
	LOGLEVEL_ERROR,
	LOGLEVEL_WARN,
	LOGLEVEL_INFO,
	LOGLEVEL_DEBUG
};

enum ZC_AUTH_TYPE_ {
	ZC_AUTH_TYPE_OPEN,
	ZC_AUTH_TYPE_SHARED,
	ZC_AUTH_TYPE_WPAPSK,
	ZC_AUTH_TYPE_WPA8021X,
	ZC_AUTH_TYPE_WPA2PSK,
	ZC_AUTH_TYPE_WPA28021X,
	ZC_AUTH_TYPE_WPAPSKWPA2PSK,
	ZC_AUTH_TYPE_MAX = ZC_AUTH_TYPE_WPAPSKWPA2PSK,
	ZC_AUTH_TYPE_INVALID = 0xff,
};

enum _ENC_TYPE_ {
	ZC_ENC_TYPE_NONE,
	ZC_ENC_TYPE_WEP,
	ZC_ENC_TYPE_TKIP,
	ZC_ENC_TYPE_AES,//CCMP
	ZC_ENC_TYPE_TKIPAES,
	ZC_ENC_TYPE_MAX = ZC_ENC_TYPE_TKIPAES,
	ZC_ENC_TYPE_INVALID = 0xff,
};

struct zconfig_constructor {
	// get system time, in ms.
	unsigned int (*zc_get_time)(void);
	
	//malloc & free
	void *(*zc_malloc)(int size);
	void (*zc_free)(void *ptr);
	
	//tpsk, released by alibaba
	/* exmaple
		char *vendor_get_tpsk(void)
		{
			return "16UQNY5bOiKG4qtjbCMnTeQXziJ9E6yq9zLMZzbnnrY=";	//test tpsk
		}	
	*/
	char *(*zc_get_tpsk)(void);
	
	/* log func, log level see enum _LOGLEVEL_, see the following exmaple
	#include <stdarg.h>
	void vendor_printf(int log_level, const char* log_tag,
	const char* file, const char* fun, int line, const char* fmt, ...)
	{
		char msg[256];
		va_list args;
		
		if (log_level > LOGLEVEL_INFO)
		return;
		
		va_start(args, fmt);
		vsnprintf(msg, sizeof(msg), fmt, args);
		va_end(args);

		printf("%s", msg);
	}
	*/
	void (*zc_printf)(int log_level, const char* log_tag, const char* file,
			  const char* fun, int line, const char* fmt, ...);

	/*
	事件通知：主要包含两个事件
	a) 锁定信道事件
	b) 接收到ssid和passwd事件

	事件可基于callback实现或通过通过zconfig_recv_callback()的返回值来判断事件
	*/
	void (*zc_cb_channel_locked)(u8 channel);
	/*
	callback, 用于返回配网结果
	其中ssid, passwd一定会返回, bssid, auth, encry可能为NULL, ZC_AUTH_TYPE_INVALID, ZC_ENC_TYPE_INVALID
	*/
	void (*zc_cb_got_ssid_passwd)(u8 *ssid, u8 *passwd, u8 *bssid, u8 auth, u8 encry, u8 channel);
};

enum _PKG_TYPE_ {
	PKG_INVALID,		//invalid pkg, --无效包
	PKG_BC_FRAME,		//broadcast frame, --信道扫描阶段，收到收到该返回值建议延长在当前信道停留时间，可以延长T1
	PKG_START_FRAME,	//start frame, --信道扫描阶段，收到该返回值用于锁定信道
	PKG_DATA_FRAME,		//data frame, --数据包，锁定信道后长时间T2收不到数据包，需重新进入扫描阶段
	PKG_ALINK_ROUTER,	//alink router
	PKG_GROUP_FRAME,	//group frame
	PKG_END			//--配网结束事件，已拿到ssid和passwd，通过回调函数去获取ssid和passwd
	/*
		参考值：
		T1:             400ms >= T2 >= 100ms
		T2:             3s
	*/
};


//进入monitor模式前后调用该函数
void zconfig_init(struct zconfig_constructor *con);
//配网成功后，调用该函数，释放内存资源
void zconfig_destroy(void);
/*
	进入monitor/sniffer模式后，将收到的包传给该函数进行处理
	若进入monitor时进行包过滤配置，以下几种包不能过滤：
	1) 数据包，目的地址为广播地址
	2) 长度>40的管理帧
	厂家需要自行切换信道，切信道时间按照自身平台需要进行调整，建议取值范围[100ms - 300ms]
	其中，6号信道需作为固定信道放在信道列表里！！！
	input:
	pkt_data: 80211 wireless raw package, include data frame & management frame
	pkt_length:	radio_hdr + 80211 hdr + payload, without fcs(4B)
	return:
	见enum _PKG_TYPE_结构体说明
*/
int zconfig_recv_callback(void *pkt_data, u32 pkt_length, u8 channel);

/*
 * save apinfo
 * 0 -- success, otherwise, failed.
 */
int zconfig_set_apinfo(u8 *ssid, u8* bssid, u8 channel, u8 auth, u8 encry);

/* helper function, auth/encry type to string */
const char *zconfig_auth_str(u8 auth);
const char *zconfig_encry_str(u8 encry);
#endif
