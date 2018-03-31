/*
 * bbc_sdk.h
 *
 *  Created on: 2016年5月24日
 *      Author: Administrator
 */

#ifndef INCLUDES_BBC_SDK_H_
#define INCLUDES_BBC_SDK_H_

#include "cJSON.h"

//#define LICENCE "dbf184abd417aa6af32a0f3648cf0293e9bc1c3f6b3e940d9081da87a5986d906f5f77bb41a1e109"//young
#define LICENCE "b17d7cd014ab7a5ae4e287cfc4f790bee8a248eca7fca900c80c547e1f19de03b65d1d37ebd1f0bd"

#define SDK_VERSION "0.6.3"
#define SERVER_URI "http://api.bigbigcloud.cn/dh/v2/rest"

typedef struct DeviceClass{
	char name[32];  //设备类型名
}DeviceClass;

typedef struct Device{
	char deviceId[32];  //厂商分配给每个设备的唯一ID
	char name[32];
	char mac[32];      //设备mac地址
	char vendor[32];    //设备的厂商标识
	char firmwareVersion[32]; //固件版本号，以“xxx.xxx.xxx"形式表示
	char sdkVersion[32];  //sdk 版本
	char romType[32];   //固件版本类型，分 STABLE、DEV 两种类型
	DeviceClass deviceClass;
}Device;

//ota升级失败信息
typedef struct OtaFailedInfo{
	char curVersion[16];//当前版本号
	int errorCode;//错误码
}OtaFailedInfo;

typedef struct OtaUpdateInfo{
	char packUrl[128];
	char packMD5[64];
	char romType[16];
	int  packSize;
	char feature[512];
	char detailDesc[1024];
	char curVersion[16];
	char pubTime[64];
	char updatePrompt[1024];
	int  packType;
	char newVersion[16];
}OtaUpdateInfo;

//设备注册
char* register_device(Device *device);

//设备信息同步
int sync_device(char* deviceGuid, Device *device, OtaFailedInfo *failedInfo);

//OTA 升级查询
OtaUpdateInfo* check_otaupdate(const char* deviceGuid, Device *device, int64_t*  nextQueryTime);

#endif /* INCLUDES_BBC_SDK_H_ */
