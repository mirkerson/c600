/*
 * main.c
 *
 *  Created on: 2016年5月27日
 *      Author: Thomas.yang
 */


#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <netdb.h>
#include <time.h>
#include <sys/stat.h> 

#include "bbc_sdk.h"
#include "cJSON.h"
#include "http_connect.h"
#include "dbg.h"
#include "property.h"
#include "utils.h"
#include "ad_type.h"

#define OTA_PATH    "/mnt/sdcard/ota_update_allwinnertech"
#define KER_IMG     "uImage"
#define ROOTFS_IMG  "rootfs.squashfs"


extern unsigned char *SHA1(const unsigned char *d, size_t n, unsigned char *md);

//设备信息，根据设备具体信息赋值
Device device = {
		.deviceId = "20160715",
		.name = "c600",
		.mac = "cda03200ab",
		.vendor = "allwinnerotatest1",
		.firmwareVersion = "0.0.10",
		.romType = "DEV",
		.sdkVersion = SDK_VERSION,
		.deviceClass = {
			.name = "c600"
		}
};

//设备升级失败的上报信息
OtaFailedInfo failedInfo = {
		.curVersion = "0.0.1",
		.errorCode = 101
};

//需要存储的一些设置项，目前有"register"(是否注册），"timestamp" (上次接收命令的时间戳），"version"(旧版本信息）
cJSON* config = NULL;

factory_func_callback *ffc_ota;

void ota_set_external_callback(void *ffc)
{
    ffc_ota = (factory_func_callback *)ffc;
}

void get_firm_version(char *dst)
{
	FILE *fp;
	int i;
	char buf[32];
	
	if((fp=fopen("/tmp/conf/config.dat","r"))!=NULL)
	{
		for(i=1;i<15;i++)
		{
		    memset(buf,0x0,sizeof(buf));
            
			fgets(buf,sizeof(buf),fp);
		}

        fgets(dst, 32, fp);

		for(i=0;i<32;i++)
		{
		    if('\n' == dst[i])
			{
				dst[i] = '\0';
				break;
			}
		}
		        
		fclose(fp);
	}

}

void set_firm_version(char *src)
{
    char cmd[64];
    
    sprintf(cmd ,"/usr/sbin/save.sh 9 %s", src);    
    system(cmd);
}

int ota_check_firm_version(update_info *info)
{
	OtaUpdateInfo* updateInfo;
	int64_t nextQueryTime;
    
	config = initProperty();
	
	char* deviceGuid = getDeviceGuid(config);
	if(config == NULL){
		debug(" init property failed ");
		return -1;
	}

    get_firm_version(device.firmwareVersion);
    printf("curren version: %s\n", device.firmwareVersion);
   
	if (deviceGuid == NULL)
	{
		deviceGuid = register_device(&device);
		if(deviceGuid == NULL){
			printf("register_device error !!!");
			return -1;
		}
            
		setDeviceGuid(config, deviceGuid);
	}
    
	if (!sync_device(deviceGuid,&device, &failedInfo))
	{
		printf("sync device error !!!");
		return -1;
	}
    
    updateInfo = check_otaupdate(deviceGuid, &device, &nextQueryTime);
    if(NULL == updateInfo)
    {
        perror("updateInfo error!");
        return -1;
    }

    if(updateInfo->packSize <= 0)
    {
        perror("The version is newest!");

        strcpy(info->firm_version, device.firmwareVersion);
        
        free(updateInfo);
        
        return 1;
    }

    strcpy(info->url, updateInfo->packUrl);
    strcpy(info->firm_version, updateInfo->newVersion);
    
    printf("have new version to update--------------\n");    

    free(updateInfo);
    
	return 0;

}

#if 1
int ota_update(update_info *info)
{
    char cmd[256];
    
    sprintf(cmd ,"rm -rf %s", OTA_PATH);    
    system(cmd);

    if(-1 == mkdir(OTA_PATH, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
    {
        ffc_ota->player.tone_play("/home/ad/qingchaka.mp3", 0);       
        return -1;
    }
    
    asyn_download(info->url, OTA_PATH);

    printf("\ndownload img finish!!\n");

    if( access(OTA_PATH, 0) < 0 )
    {
        return -1;
    }        

    set_firm_version(info->firm_version);

    return 0;
    
}

#else
int ota_update(char *version)
{
//	char cmd[64] = {0};
	OtaUpdateInfo* updateInfo;
	int64_t nextQueryTime;
    
	config = initProperty();//初始化配置项
	
	char* deviceGuid = getDeviceGuid(config);
	if(config == NULL){
		debug(" init property failed ");
		return -1;
	}

    get_firm_version(device.firmwareVersion);
    printf("curren version: %s\n", device.firmwareVersion);

    strcpy(version, device.firmwareVersion);

	//设备第一次启动时注册设备
	if (deviceGuid == NULL)
	{
		deviceGuid = register_device(&device);
		if(deviceGuid == NULL){
			printf("register_device error !!!");
			return -1;
		}
            
		setDeviceGuid(config, deviceGuid);
	}
    
	//每次设备启动，都需要同步设备信息
	if (!sync_device(deviceGuid,&device, &failedInfo))
	{
		printf("sync device error !!!");
		return -1;
	}

#if 0
	//查询更新命令，输入：checkUpdate  查询更新，  查到更新后，输入：download，下载更新
	while(1){
		scanf("%s", cmd);
		if (strcmp(cmd,"checkUpdate") == 0){
			updateInfo = check_otaupdate(deviceGuid, &device, &nextQueryTime);
			if(updateInfo != NULL){
				printf("packUrl %s\n"
						"packMD5 %s\n"
						"romType %s\n"
						"packSize %d\n"
						"feature %s\n"
						"detailDesc %s\n"
						"curVersion %s\n"
						"pubTime %s\n"
						"updatePrompt %s\n"
						"packType %d\n"
						"newVersion %s\n", updateInfo->packUrl, updateInfo->packMD5, updateInfo->romType,
						updateInfo->packSize, updateInfo->feature, updateInfo->detailDesc, updateInfo->curVersion,
						updateInfo->pubTime, updateInfo->updatePrompt, updateInfo->packType, updateInfo->newVersion);
				printf(" next query time %lld \n", nextQueryTime);

                if(updateInfo->packSize > 0)
                {
                    printf("have new version to update--------------\n");
                }
                else
                    printf("no update found!!!\n");
                
			}else {
				printf("no update found!!!\n");
			}
		}else if(strcmp(cmd,"download") == 0){
			if(updateInfo != NULL){
				asyn_download(updateInfo->packUrl, "./update");
			}
		}
	};
#else

    
    updateInfo = check_otaupdate(deviceGuid, &device, &nextQueryTime);
    if(NULL == updateInfo)
    {
        perror("updateInfo error!");
        return -1;
    }

    if(updateInfo->packSize <= 0)
    {
        perror("The version is newest!");

        free(updateInfo);

        return -1;
    }

    printf("have new version to update--------------\n");
    
    ffc_ota->player.tone_play("/home/ad/otastart.mp3", 0);       
    
    printf("packUrl %s\n"
            "packMD5 %s\n"
            "romType %s\n"
            "packSize %d\n"
            "feature %s\n"
            "detailDesc %s\n"
            "curVersion %s\n"
            "pubTime %s\n"
            "updatePrompt %s\n"
            "packType %d\n"
            "newVersion %s\n", updateInfo->packUrl, updateInfo->packMD5, updateInfo->romType,
            updateInfo->packSize, updateInfo->feature, updateInfo->detailDesc, updateInfo->curVersion,
            updateInfo->pubTime, updateInfo->updatePrompt, updateInfo->packType, updateInfo->newVersion);
    
    printf(" next query time %lld \n", nextQueryTime);

    asyn_download(updateInfo->packUrl, OTA_PATH);
    
    printf("\ndownload img finish!!\n");

    if( access(OTA_PATH, 0) < 0 )
    {
        free(updateInfo);
        return -1;
    }
        
    
    set_firm_version(updateInfo->newVersion);
            
#endif

    free(updateInfo);

	//cJSON_Delete(config);
 
	return 0;

}
#endif

void flash_burn(void)
{
    char cmd[256];
    char file_path[256];
   
    sprintf(cmd ,"unzip %s/*.zip -d %s", OTA_PATH, OTA_PATH);    
    system(cmd);    

    //copy from rootfs, or download from http server
    sprintf(cmd ,"cp /bin/busybox %s", OTA_PATH);    
    system(cmd);

    sprintf(cmd ,"cp /usr/sbin/flash_eraseall %s", OTA_PATH);    
    system(cmd);

    sprintf(cmd ,"cp /home/tinyplay %s", OTA_PATH);    
    system(cmd);

    sprintf(cmd ,"cp /home/ad/xitongchongqi.wav %s", OTA_PATH);    
    system(cmd);

    memset(file_path, 0, sizeof(file_path));
    sprintf(file_path ,"%s/%s", OTA_PATH, KER_IMG);     
    if( 0 == access(file_path, 0) )
    {
        sprintf(cmd ,"%s/flash_eraseall /dev/mtd1", OTA_PATH);    
        system(cmd);  

        sprintf(cmd ,"%s/busybox cat %s > /dev/mtd1", OTA_PATH, file_path);    
        system(cmd);  

    }

    memset(file_path, 0, sizeof(file_path));
    sprintf(file_path ,"%s/%s", OTA_PATH, ROOTFS_IMG);     
    if( 0 == access(file_path, 0) )
    {
        sprintf(cmd ,"%s/flash_eraseall /dev/mtd2", OTA_PATH);    
        system(cmd);  

        sprintf(cmd ,"%s/busybox cat %s > /dev/mtd2", OTA_PATH, file_path);    
        system(cmd);  

    }    

    sprintf(cmd ,"%s/tinyplay %s/xitongchongqi.wav", OTA_PATH, OTA_PATH);    
    system(cmd);

    sprintf(cmd ,"%s/busybox reboot", OTA_PATH);    
    system(cmd);

    
}

