/*
 * bbc_sdk.c
 *
 *  Created on: 2016年5月24日
 *      Author: Thomas
 */

#include <stdlib.h>
#include <stdio.h>
#include<malloc.h>
#include <netdb.h>
#include <time.h>
#include "bbc_sdk.h"
#include "utils.h"
#include "cJSON.h"
#include "http_connect.h"
#include "dbg.h"

cJSON* toJson(Device* device){
	   cJSON *jDevice = cJSON_CreateObject();
	   cJSON *jDeviceClass = cJSON_CreateObject();

	   //deviceClass
		cJSON_AddItemToObject(jDeviceClass,"name", cJSON_CreateString(device->deviceClass.name));
	   //device
	   cJSON_AddItemToObject(jDevice,"deviceId", cJSON_CreateString(device->deviceId));// <=== id
	   cJSON_AddItemToObject(jDevice,"name",cJSON_CreateString(device->name));
	   cJSON_AddItemToObject(jDevice,"mac",cJSON_CreateString(device->mac));
	   cJSON_AddItemToObject(jDevice,"vendor",cJSON_CreateString(device->vendor));
	   cJSON_AddItemToObject(jDevice,"firmwareVersion",cJSON_CreateString(device->firmwareVersion));
	   cJSON_AddItemToObject(jDevice,"sdkVersion",cJSON_CreateString(device->sdkVersion));
	   cJSON_AddItemToObject(jDevice,"romType",cJSON_CreateString(device->romType));
	   cJSON_AddItemToObject(jDevice,"deviceClass",jDeviceClass);
	   return jDevice;
}

//成功 ：返回deviceGuid，失败返回 NULL
char* register_device(Device *device){
	char* result = NULL;
	char temp_url[64] = {0};
	char temp_signature[256] = {0};
	char digest[64] = {0};
	char signature[64] = {0};
	char request[1024] = {0};
	char* response = NULL;
	int len = 0;
	time_t  time_of_seconds = 0;
	Url* url = NULL;
	cJSON* json_response = NULL, *status = NULL,*data = NULL, *guid = NULL;
	cJSON* jdevice = toJson(device);
	char* device_str = cJSON_PrintUnformatted(jdevice);
	sprintf(temp_url, "%s%s",SERVER_URI, "/device");
	url = url_parse(temp_url);
	if(url == NULL){
		printf(" parse url error ");
		goto error;
	}
	//生成签名 组成请求内容
	time(&time_of_seconds); //签名的时间戳，单位：秒
	printf(" time of seconds %ld ,", time_of_seconds);
	len = sprintf(temp_signature, "%s%s%s%s%ld%s", device->deviceId, device->mac, device->vendor,device->deviceClass.name,time_of_seconds,LICENCE);
	temp_signature[len] = '\0';
	printf(" len %d temp_signature %s \n", len, temp_signature);
	SHA1((unsigned char*)temp_signature, len, (unsigned char*)digest);
	to_hex_str((unsigned char*)digest, signature, 20);
	printf(" device str %s , signature %s \n", device_str, signature);
	sprintf(request, "PUT %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"User-Agent: allwinnertech\r\n"
		"Range: bytes=0-\r\n"
		"Content-Type: application/json;charset=utf-8\r\n"
		"Content-Length: %d\r\n"
		"Auth-Timestamp: %ld\r\n"
		"Auth-Signature: %s\r\n"
		"Connection: Close\r\n\r\n"
		"%s\r\n",
		url->path, url->hostname, strlen(device_str),time_of_seconds, signature, device_str);
	printf(" send request %s \n", request);

	response  = execute_request(url->hostname, url->port, request);
	//printf("response %s ", response);
	if(response == NULL){
		printf("response error ");
		goto error;
	}
    json_response = cJSON_Parse(response);
    if(json_response == NULL){
    	goto error;
    }
    printf("%s %d\n", __FILE__,__LINE__);
	status = cJSON_GetObjectItem(json_response,"status");
	if(status == NULL || strcmp("error", status->valuestring) == 0){
		goto error;
	}
        printf("%s %d status->valuestring=%s\n", __FILE__,__LINE__,status->valuestring);
	data = cJSON_GetObjectItem(json_response,"data");
    
	if(data != NULL){
		guid = cJSON_GetObjectItem(data, "deviceGuid");
		if(guid == NULL){
			goto error;
		}
		result = malloc(sizeof(char) * ( strlen(guid->valuestring) + 1 ));

        printf("%s %d guid->valuestring=%s\n", __FILE__,__LINE__,guid->valuestring);
        printf("%s %d strlen(guid->valuestring)=%d sizeof(guid->valuestring)=%d\n", __FILE__,__LINE__,strlen(guid->valuestring), sizeof(guid->valuestring));

        
		if(result != NULL){
			strcpy(result, guid->valuestring);
		}
	}
        printf("%s %d \n", __FILE__,__LINE__);
error:
	if(json_response) cJSON_Delete(json_response);
	if(jdevice)  cJSON_Delete(jdevice);
	if(url) url_free(url);
	if(response) free(response);
	if(device_str) free(device_str);
	return result;
}

//返回 0：失败，  1： 成功
int sync_device(char* deviceGuid, Device *device, OtaFailedInfo *failedInfo){
	int result = 0;
	char temp_url[128] = {0};
	char temp_signature[128] = {0};
	char digest[64] = {0};
	char signature[64] = {0};
	char request[1024] = {0};
	char* response = NULL;
	char* device_str = NULL;
	int len = 0;
	time_t  time_of_seconds = 0;
	Url* url = NULL;
	cJSON* json_response = NULL,*status = NULL, *ws_url = NULL, *jdevice = NULL;

    printf("%s %d---device_str=%s\n", __FILE__,__LINE__,deviceGuid);
    
	jdevice = toJson(device);
	cJSON_AddItemToObject(jdevice, "deviceGuid", cJSON_CreateString(deviceGuid));
	if(failedInfo != NULL){
		cJSON* data = cJSON_CreateObject();
		cJSON* upgradefail = cJSON_CreateObject();
		cJSON_AddItemToObject(upgradefail, "curVersion", cJSON_CreateString(failedInfo->curVersion));
		cJSON_AddItemToObject(upgradefail, "errcode", cJSON_CreateNumber(failedInfo->errorCode));
		cJSON_AddItemToObject(data, "upgradefail", upgradefail);
		cJSON_AddItemToObject(jdevice, "data", data);
	}
	device_str = cJSON_PrintUnformatted(jdevice);//需要同步的设备信息

    printf("%s %d---device_str=%s\n", __FILE__,__LINE__,device_str);

	sprintf(temp_url, "%s%s%s",SERVER_URI, "/device/", deviceGuid);
    
    printf("%s %d---device_str=%s\n", __FILE__,__LINE__,deviceGuid);
    printf("%s %d---temp_url=%s\n", __FILE__,__LINE__,temp_url);

    url = url_parse(temp_url);
	if(url == NULL){
		printf(" parse url error ");
		goto error;
	}
	//生成签名 组成请求内容
	time(&time_of_seconds);
	printf(" time of seconds %ld ", time_of_seconds);
	len = sprintf(temp_signature, "%s%ld%s", deviceGuid, time_of_seconds, LICENCE);
	SHA1(temp_signature, len, digest);
	to_hex_str((unsigned char*)digest,signature, 20);
	sprintf(request, "POST %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"User-Agent: allwinnertech\r\n"
		"Range: bytes=0-\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: %d\r\n"
		"Connection: Close\r\n"
		"Auth-Timestamp: %ld\r\n"
		"Auth-Signature: %s\r\n\r\n"
		"%s\r\n",
		url->path, url->hostname, strlen(device_str),time_of_seconds, signature, device_str);
	printf(" send request %s \n", request);
    printf("%s %d==url->hostname=%s, url->port=%s\n", __FILE__,__LINE__,url->hostname, url->port);
	response  = execute_request(url->hostname, url->port, request);
    printf("%s %d\n", __FILE__,__LINE__);
	printf("response %s \n", response);
	if(response == NULL){
		printf("response error ");
		goto error;
	}
    printf("%s %d\n", __FILE__,__LINE__);
	json_response = cJSON_Parse(response);
	if(json_response == NULL){
		printf("json_response error ");
		goto error;
	}
    printf("%s %d\n", __FILE__,__LINE__);
	status = cJSON_GetObjectItem(json_response,"status");
    printf("%s %d\n", __FILE__,__LINE__);
	if(status != NULL && strcmp("success", status->valuestring) == 0){
		result = 1;
	}
    printf("%s %d\n", __FILE__,__LINE__);
error:
    printf("%s %d\n", __FILE__,__LINE__);
	if(json_response) cJSON_Delete(json_response);
	if(jdevice)  cJSON_Delete(jdevice);
	if(url) url_free(url);
	if(response) free(response);
	if(device_str) free(device_str);
    printf("%s %d\n", __FILE__,__LINE__);
	return result;
}

OtaUpdateInfo* check_otaupdate(const char* deviceGuid, Device *device, int64_t*  nextQueryTime){
	OtaUpdateInfo* result = NULL;
	char temp_url[128] = {0};
	char temp_signature[256] = {0};
	char digest[64] = {0};
	char signature[64] = {0};
	char request[1024] = {0};
	char* response = NULL;
	int len = 0;
	time_t  time_of_seconds = 0;
	Url* url = NULL;
	cJSON* json_response = NULL, *status = NULL,*data = NULL, *guid = NULL;
	cJSON *jData = cJSON_CreateObject();
	cJSON *jDeviceClass = cJSON_CreateObject();
	cJSON_AddItemToObject(jDeviceClass,"name", cJSON_CreateString(device->deviceClass.name));
	cJSON_AddItemToObject(jData,"deviceId", cJSON_CreateString(device->deviceId));// <=== id
	cJSON_AddItemToObject(jData,"curVersion",cJSON_CreateString(device->firmwareVersion));
	cJSON_AddItemToObject(jData,"mac",cJSON_CreateString(device->mac));
	cJSON_AddItemToObject(jData,"vendor",cJSON_CreateString(device->vendor));
	cJSON_AddItemToObject(jData,"romType",cJSON_CreateString(device->romType));
	cJSON_AddItemToObject(jData,"deviceClass",jDeviceClass);

	char* data_str = cJSON_PrintUnformatted(jData);

	sprintf(temp_url, "%s/device/%s/otaupdate",SERVER_URI, deviceGuid);
	url = url_parse(temp_url);
	if(url == NULL){
		printf(" parse url error ");
		goto error;
	}
	//生成签名 组成请求内容
	time(&time_of_seconds);
	printf(" time of seconds %ld ", time_of_seconds);
	len = sprintf(temp_signature, "%s%ld%s", deviceGuid, time_of_seconds, LICENCE);
	SHA1(temp_signature, len, digest);
	to_hex_str((unsigned char*)digest,signature, 20);
	sprintf(request, "POST %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"User-Agent: allwinnertech\r\n"
		"Range: bytes=0-\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: %d\r\n"
		"Connection: Close\r\n"
		"Auth-Timestamp: %ld\r\n"
		"Auth-Signature: %s\r\n\r\n"
		"%s\r\n",
		url->path, url->hostname, strlen(data_str),time_of_seconds, signature, data_str);
	printf(" send request %s ", request);

	response  = execute_request(url->hostname, url->port, request);
	printf("response %s ", response);
	if(response == NULL){
		printf("response error ");
		goto error;
	}
	json_response = cJSON_Parse(response);
	if(json_response == NULL){
		printf("json_response error ");
		goto error;
	}
	cJSON* nextCheckTime = cJSON_GetObjectItem(json_response,"nextQueryTime");
	if(nextCheckTime != NULL){
		*nextQueryTime = (int64_t)nextCheckTime->valuedouble;
	}
	status = cJSON_GetObjectItem(json_response,"status");
	if(status != NULL && strcmp("success", status->valuestring) == 0){
		result = (OtaUpdateInfo*)malloc(sizeof(OtaUpdateInfo));
		memset(result, 0, sizeof(OtaUpdateInfo));
		cJSON* data = cJSON_GetObjectItem(json_response,"data");
		if(data != NULL){
			cJSON* packUrl = cJSON_GetObjectItem(data,"packUrl");
			if(packUrl != NULL && packUrl->valuestring != NULL){
				strcpy(result->packUrl, packUrl->valuestring);
			}
			cJSON* packMD5 = cJSON_GetObjectItem(data,"packMD5");
			if(packMD5 != NULL && packMD5->valuestring != NULL){
				strcpy(result->packMD5, packMD5->valuestring);
			}
			cJSON* romType = cJSON_GetObjectItem(data,"romType");
			if(romType != NULL && romType->valuestring != NULL){
				strcpy(result->romType, romType->valuestring);
			}
			cJSON* packSize = cJSON_GetObjectItem(data,"packSize");
			if(packSize != NULL){
				result->packSize = packSize->valueint;
			}
			cJSON* feature = cJSON_GetObjectItem(data,"feature");
			if(feature != NULL && feature->valuestring != NULL){
				strcpy(result->feature, feature->valuestring);
			}
			cJSON* detailDesc = cJSON_GetObjectItem(data,"detailDesc");
			if(detailDesc != NULL && detailDesc->valuestring != NULL){
				strcpy(result->detailDesc, detailDesc->valuestring);
			}
			cJSON* curVersion = cJSON_GetObjectItem(data,"curVersion");
			if(curVersion != NULL && curVersion->valuestring != NULL){
				strcpy(result->curVersion, curVersion->valuestring);
			}
			cJSON* pubTime = cJSON_GetObjectItem(data,"pubTime");
			if(pubTime != NULL && pubTime->valuestring != NULL){
				strcpy(result->pubTime, pubTime->valuestring);
			}
			cJSON* updatePrompt = cJSON_GetObjectItem(data,"updatePrompt");
			if(updatePrompt != NULL && updatePrompt->valuestring != NULL){
				strcpy(result->updatePrompt, updatePrompt->valuestring);
			}
			cJSON* packType = cJSON_GetObjectItem(data,"packType");
			if(packType != NULL){
				result->packType = packType->valueint;
			}
			cJSON* newVersion = cJSON_GetObjectItem(data,"newVersion");
			if(newVersion != NULL && newVersion->valuestring != NULL){
				strcpy(result->newVersion, newVersion->valuestring);
			}
		}
	}
error:
	if(json_response) cJSON_Delete(json_response);
	if(jData)  cJSON_Delete(jData);
	if(url) url_free(url);
	if(response) free(response);
	if(data_str) free(data_str);
	return result;
}
