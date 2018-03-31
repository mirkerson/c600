/*
 * property.c
 *
 *  Created on: 2015年12月18日
 *      Author: Administrator
 */

#include "property.h"
#include "dbg.h"
cJSON*  initProperty(){
	char* str;
	int len;
	cJSON* config = NULL;
	FILE* fp = fopen(PROPERTY_FILE, "r");
	if(fp == NULL) {
		config = cJSON_CreateObject();
	}else{
		fseek(fp,0L,SEEK_END);
		len = ftell(fp);
		debug(" len %i \n", len);
		str = (char*)malloc(len + 1);
		if(str == NULL) {
			fclose(fp);
			return NULL;
		}
		fseek(fp,0L,SEEK_SET);
		fread(str, len, 1, fp);
		str[len] = 0;
		debug(" init file string %s \n", str);
		config = cJSON_Parse(str);
		fclose(fp);
	}
	return config;
}

void saveProperty(cJSON* config){
	FILE* fp;
	char* str = cJSON_PrintUnformatted(config);
	debug(" save %s ", str);
	fp = fopen(PROPERTY_FILE, "w+");
	if(fp == NULL) return;
	//fprintf(fp, "%s", str);
	fwrite(str, 1, strlen(str), fp);
	fclose(fp);
}


void setDeviceGuid(cJSON* config, char* deviceGuid){
	cJSON* item = cJSON_GetObjectItem(config, "deviceGuid");
	if(item != NULL){
		cJSON_DeleteItemFromObject(config,"deviceGuid");
	}
	cJSON_AddItemToObject(config, "deviceGuid", cJSON_CreateString(deviceGuid));
	saveProperty(config);
}

char* getDeviceGuid(cJSON* config){
	cJSON* item = cJSON_GetObjectItem(config, "deviceGuid");
	char* deviceGuid;
	if(item != NULL && item->valuestring != NULL){
		deviceGuid = malloc(sizeof(char)*( strlen(item->valuestring) + 1 ));
		memset(deviceGuid, 0, strlen(item->valuestring));
		strcpy(deviceGuid, item->valuestring);
        
        return deviceGuid;
	}
    else
        return NULL;
}
