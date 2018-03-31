/**
 * @file swift_link.h
 * @brief  Get AP's ssid, password, encryption method API
 * @author Zac, <zhangchao@allwinnertech.com>
 * @version 1.4
 * @date 2016-09-21
 *
 * Copyright (C) 2016 Allwinner Technology CO., Ltd.
 *
 * The program is not free, Allwinner without permission,
 * no one shall be arbitrarily (including but not limited
 * to: copy, to the illegal way of communication, display,
 * mirror, upload, download) use, or by unconventional
 * methods (such as: malicious intervention Allwinner data)
 * Allwinner's normal service, no one shall be arbitrarily by
 * software the program automatically get Allwinner data
 * Otherwise, Allwinner will be investigated for legal responsibility
 * according to law.
 */
#ifndef __SWIFT_LINK_H__
#define __SWIFT_LINK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <alsa/asoundlib.h>


/**
 * @brief system readable and writeable directory.
 */
#define	RWDIR                       "/tmp"

/**
 * @brief AP encryption method is WPA the key word.
 */
#define ENCRYPT_TYPE_WPA_KEY		"#!#"

/**
 * @brief AP encryption method is WEP the key word.
 */
#define ENCRYPT_TYPE_WEP_KEY		"#$#"

/**
 * @brief iOS the key word.
 */
#define ADT_iOS_KEY		            "#&#"

extern pthread_mutex_t decode_return_lock;
/**
 * @brief wifi config method.
 */
typedef enum netcfg_method {
    ADT = 0x01,                     /**< adt method for Allwinner. */
    SMART_CONFIG = 0x02,            /**< smart config method for Xinzhilian. */
    SWIFT_LINK = 0x03               /**< swift link method for Mix. */
} netcfg_method_t;

/**
 * @brief swift link return state.
 */
typedef enum swift_state {
    DEC_ERR = -1,                   /**< swift link return error. */
    DEC_ADT_NORM,                   /**< swift link adt decode return normal. */
    DEC_SMT_NORM,                   /**< swift link smart config decode return normal. */
    DEC_FC,                         /**< swift link return force close. */
    DEC_TO                          /**< swift link return timeout. */
} swift_state_t;

/**
 * @brief encryption method.
 */
typedef enum key_mgmt {
    SECURITY_WPA,                   /**< encryption method is WPA type */
    SECURITY_WEP,                   /**< encryption method is WEP type */
    SECURITY_NONE,                  /**< encryption method is not used */
    SECURITY_INV                    /**< encryption method is invalid */
} key_mgmt_t;

/**
 * @brief string format of encryption method, for readable.
 */
extern const char *key_mgmt_str[];

/**
 * @brief audio device info set.
 */
typedef struct device_info {
    /**
     * @brief audio device name.
     */
    char *devicename;
    /**
     * @brief audio sampling rate.
     */
    unsigned int rate;
    /**
     * @brief audio channel number.
     */
    unsigned int channels;
    /**
     * @brief audio sampling number.
     */
    snd_pcm_format_t bits;
} device_info_t;

typedef struct param_info {
    /**
     * @brief adt decode timeout. (default 60s)
     */
    int adt_timeout;
    /**
     * @brief smart config decode timeout. (default 60s)
     */
    int smt_timeout;
    /**
     * @brief audio device info.
     */
    device_info_t dev_info;
} param_info_t;

/**
 * @brief swift link return success result.
 */
typedef struct swift_link_result {
    /**
     * @brief ssid.
     */
    char ssid[32 + 1];
    /**
     * @brief password.
     */
    char passwd[64 + 1];
    /**
     * @brief ssid length.
     */
    char ssid_length;
    /**
     * @brief password length.
     */
    char passwd_length;
    /**
     * @brief encryption method. (see key_mgmt_t)
     */
    key_mgmt_t encrypt_type;
} swift_link_result_t;

/**
 * @brief swift link start.
 *
 * @param swift_result  [out] return decode result.
 * @param param_info    [in] pass parameters.
 * @param cfg_method    [in] wifi config method. (see netcfg_method_t)
 *
 * @return rerurn swift link state. (see swift_state_t)
 */
extern swift_state_t swift_decode_start(swift_link_result_t *swift_result, param_info_t param_info, netcfg_method_t cfg_method);
/**
 * @brief swift link shutdown.
 *
 * @param cfg_method    [in] wifi config method.
 */
extern void swift_decode_shutdown(netcfg_method_t cfg_method);
/**
 * @brief swift link reset. (Paired with shutdown)
 *
 * @param cfg_method    [in] wifi config method.
 */
extern void swift_decode_reset(netcfg_method_t cfg_method);

#ifdef __cplusplus
}
#endif

#endif /* __SWIFT_LINK_H__ */
