#ifndef __HW_H
#define __HW_H

typedef enum
{
	LED_WIFI_CONNECTED = 1,
	LED_WIFI_CONNECTING,
	LED_WIFI_DISCONNECT,
	LED_WIFI_DEFAULT	
} ;


typedef enum
{
	PIN_DIR_INPUT,
	PIN_DIR_OUTPUT,
	PIN_DIR_DEFAULT
} pin_dir_t;

typedef	enum
{
	GPIO_NODE_MUL_SEL		=	0x00,
	GPIO_NODE_PULL			=	0x01,
	GPIO_NODE_DRV_LEVEL		=	0x02,
	GPIO_NODE_DATA			=	0x03,
	GPIO_NODE_DEFAULT
} gpio_node_t;

typedef enum
{
	PIN_PULL_DISABLE 	=	0x00,
	PIN_PULL_UP		=	0x01,
	PIN_PULL_DOWN	  	=	0x02,
	PIN_PULL_RESERVED	=	0x03,
	PIN_PULL_DEFAULT
} pin_pull_level_t;

typedef	enum
{
	PIN_MULTI_DRIVING_0			=	0x00,
	PIN_MULTI_DRIVING_1			=	0x01,
	PIN_MULTI_DRIVING_2			=	0x02,
	PIN_MULTI_DRIVING_3			=	0x03,
	PIN_MULTI_DRIVING_DEFAULT
} pin_drive_level_t;

typedef enum
{
	PIN_DATA_LOW,
	PIN_DATA_HIGH,
	PIN_DATA_DEFAULT
} pin_data_t;

/* bt operations */
typedef enum
{
	BT_DISCONNECT,
	BT_CONNECTED,
	BT_A2DP
} bt_status_t;




#endif

