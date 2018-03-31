/*
 * property.h
 *
 *  Created on: 2015年12月18日
 *      Author: Administrator
 */

#ifndef INCLUDES_PROPERTY_H_
#define INCLUDES_PROPERTY_H_

#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define PROPERTY_FILE "./mydevice.property"

cJSON*  initProperty();
void saveProperty(cJSON* config);
void setRegistered(cJSON* config,int is_registered);//保存设备是否已注册标志
int getRegistered(cJSON* config);
void setDeviceGuid(cJSON* config, char* deviceGuid);//保存平台分配给device的唯一id
char* getDeviceGuid(cJSON* config);

#ifdef __cplusplus
}
#endif


#endif /* INCLUDES_PROPERTY_H_ */
