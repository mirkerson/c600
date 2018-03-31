#ifndef	_ZCONFIG_VENDOR_H_
#define _ZCONFIG_VENDOR_H_

/*
 * this callback will be called when zconfig success or timeout
 * when success:
 *      this callback will be called with ssid & passwd,
 *      but bssid, security, and channel can be NULL or invalid,
 *      in this case user need to rescan the ssid to get bssid,
 *      security, channel themselves.
 * when timeout: (60s)
 *      this callback will be called with null ssid.
 */
typedef int (*zc_callback)(char *ssid, char *passwd, char *bssid, unsigned int security, char channel);


/*
 * this func will block until zconfig success or timeout(60s)
 * tpsk is released by alibaba for each alink device model.
 * when zconfig found a alink router, tpsk will be used as a factor to connect it.
 */
void zconfig_start(zc_callback cb, const char *tpsk);

/*
 * send broadcast msg to APP to stop the zconfig process.
 * this func will block 10~20s.
 * IMPORTANT:
 *      Calling this func as soon as wifi connected(dhcp ready), but
 *      if alink need to do unbind/factory_reset, calling this func after
 *      alink_wait_connect(NULL, 10), 10 means timeout 10s.
 */
void zconfig_notify_app(char *model, char *mac, char *sn);
#endif
