#ifndef  __OSAL_PARSER_H__
#define  __OSAL_PARSER_H__

#include "OSAL.h"

#define TYPE_INTEGER	1
#define TYPE_STRING	2
#define TYPE_GPIO	3

int OSAL_Script_Parser_Disp_Init_Data(char *main_name, char *sub_name, int value[], int type);
int OSAL_Script_FetchParser_Data(char *main_name, char *sub_name, int value[], int type);
int OSAL_sw_get_ic_ver(void);

#endif
