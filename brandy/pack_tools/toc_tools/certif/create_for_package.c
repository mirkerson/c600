/*
**********************************************************************************************************************
*											        eGon
*						           the Embedded GO-ON Bootloader System
*									       eGON arm boot sub-system
*
*						  Copyright(C), 2006-2014, Allwinner Technology Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      : Jerry
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include "common.h"
#include "include.h"

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int createcnf_for_package(char *lpCfg, toc_descriptor_t *package)
{
	char all_package[1024];
	int  i;
	char *all_package_line[16];
	char type_name[32], line_info[256];
	char bin_name[64];

	memset(all_package, 0, 1024);
	memset(all_package_line, 0, 16 * sizeof(char *));

	if(GetPrivateProfileSection("package", all_package, 1024, lpCfg))
	{
		printf("dragoncreate_package err in GetPrivateProfileSection\n");

		return -1;
	}
	if(all_package[0] == '\0')
	{
		printf("dragoncreate_package err no content match package\n");

		return -1;
	}

	if(GetPrivateProfileAllLine(all_package, all_package_line))
	{
		printf("dragoncreate_package err in GetPrivateProfileAllLine\n");

		return -1;
	}
	for(i=0;i<PACKAGE_CONFIG_MAX;i++)
	{
		if(all_package_line[i])
		{
			memset(type_name, 0, 32);
			memset(line_info, 0, 256);

			GetPrivateProfileLineInfo(all_package_line[i], type_name, line_info);

			memset(bin_name , 0, 64);

			GetPrivateEachItem(line_info, package[i].item, bin_name, NULL);

			package[i].type = NORMAL_TYPE;
			GetFullPath(package[i].bin, bin_name);
		}
		else
		{
			break;
		}
	}

	return 0;
}

