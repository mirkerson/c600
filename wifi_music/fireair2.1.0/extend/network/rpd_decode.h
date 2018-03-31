#ifndef __RPD_DECODE_H__
#define __RPD_DECODE_H__

#include <sys/wait.h>
#include <sys/time.h>

#ifdef DBG_EFF_
struct timeval tstart, tend, xrsc_trecv, adt_trecv, conn_tstart, conn_tend;
float timeuse;
#endif



#define DEC_ERR		-1
#define DEC_NORM	0
#define DEC_FC		1

typedef enum netcfg_method {
    ADT = 0x01,
    SMART_CONFIG = 0x02,
    SWIFT_LINK = 0x03
} netcfg_method_t;

extern int swift_decode_start(unsigned char *str, char *devicename, netcfg_method_t cfg_method);
extern void swift_decode_shutdown(netcfg_method_t cfg_method);
extern void swift_decode_reset(netcfg_method_t cfg_method);

#endif
