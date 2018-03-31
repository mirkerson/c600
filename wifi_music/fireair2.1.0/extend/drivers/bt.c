#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "common_def.h"
#include "hw.h"


#define C600 
#ifdef  C600
#define GPIO_ENABLE_NAME 	"PD5"
//#define VDD_POWER_NAME 		"ldo3"
#define GPIO_LEDA_NAME 		"Pd4"
#define GPIO_LEDB_NAME 		"PC14"
#define GPIO_CTRL_NAME 		"PD2"

#else
#define GPIO_ENABLE_NAME 	"PE4"
#define VDD_POWER_NAME 		"ldo3"
#define GPIO_LEDA_NAME 		"PC15"
#define GPIO_LEDB_NAME 		"PC14"
#define GPIO_CTRL_NAME 		"PC13"
#endif


/***************************************************************
 Function:        bt_on
 Description:	  power on BT
 Input:            N/A
 Output:            N/A
 Return:          OK/ERROR
 ***************************************************************/
int bt_on(void)
{
    int ret = OK;

	/*
    if(OK != regulator_set_volts(VDD_POWER_NAME,3300000,3300000))
    {
        ret = ERROR;
        goto exit;
    }

    usleep(100*1000); // 100 ms
    */
    //if(EGPIO_OK != gpio_set_pin_io_status(GPIO_ENABLE_NAME, PIN_DIR_OUTPUT)) {
    //    printf("[BT] Init Bt GPIO Fail!\n");
    //}

    if(OK != gpio_write_pin_value(GPIO_ENABLE_NAME, PIN_DATA_HIGH)) {
        ret = ERROR;
        goto exit;
    }

exit:
    return ret;
}

/***************************************************************
 Function:        bt_off
 Description:	  power off BT
 Input:            N/A
 Output:            N/A
 Return:          OK/ERROR
 ***************************************************************/
int bt_off(void)
{
    int ret = OK;

    if(OK != gpio_write_pin_value(GPIO_ENABLE_NAME, PIN_DATA_LOW)) {
        ret = ERROR;
        goto exit;
    }

    usleep(2000*1000); // 2000 ms
    /*
    if(OK != regulator_set_volts(VDD_POWER_NAME,0,0))
    {
        ret = ERROR;
        goto exit;
    }*/

exit:
    return ret;
}

/***************************************************************
 Function:        get_btstatus
 Description:	  get current status of BT
 Input:            N/A
 Output:            N/A
 Return:         BT_CONNECTED --> connected ;
		 BT_A2DP --> A2DP
		 <0 --> get error;
		BT_DISCONNECT : no connected and no A2DP.
 ***************************************************************/
int bt_get_ConnectedOrPlay(void)
{
    int value = 0 , value2 = 0;

    if(OK != gpio_read_pin_value(GPIO_LEDA_NAME,&value)) {
        return ERROR;
    }

    if(OK != gpio_read_pin_value(GPIO_LEDB_NAME,&value2)) {
        return ERROR;
    }

    if(value == PIN_DATA_HIGH && value2 == PIN_DATA_LOW) {
        return BT_CONNECTED;
    }

    if(value == PIN_DATA_LOW && value2 == PIN_DATA_HIGH) {
        return BT_A2DP;
    }

    //if ( (LEDA-> high && LEDB-> high) || (LEDA-> low && LEDB-> low) )
    return BT_DISCONNECT;
}

/***************************************************************
 Function:        bt_disconnect
 Description:	  disconnect current link.make BT connectable and discoverable
 Input:            N/A
 Output:            N/A
 Return:          OK/ERROR
 ***************************************************************/
int bt_disconnect(void)
{
    int ret = OK;

    if(OK != gpio_write_pin_value(GPIO_ENABLE_NAME, PIN_DATA_HIGH)) {
        ret = ERROR;
        goto exit;
    }

    usleep(3000*1000); // 3000 ms

    if(OK != gpio_write_pin_value(GPIO_ENABLE_NAME, PIN_DATA_LOW)) {
        ;//be OK.
    }

exit:
    return ret;
}

/***************************************************************
 Function:        bt_init
 Description:	  init GPIO
 Input:            N/A
 Output:            N/A
 Return:          OK/ < 0
 ***************************************************************/
int bt_init()
{
    int ret = OK;

    ret += gpio_set_pin_io_status(GPIO_ENABLE_NAME, PIN_DIR_OUTPUT);
    ret += gpio_set_pin_io_status(GPIO_LEDA_NAME,   PIN_DIR_INPUT);
    //ret += gpio_set_pin_io_status(GPIO_LEDB_NAME, PIN_DIR_INPUT);
    ret += gpio_set_pin_io_status(GPIO_CTRL_NAME,   PIN_DIR_OUTPUT);

    return ret;
}

int user_bt_init(void)
{
	#ifdef C500_BOARD
		system("echo 1 >  /sys/class/gpio/gpio101/value");
    #else
        system("echo 1 >  /sys/class/gpio/gpio130/value");
    #endif
    return 0;
}

int user_bt_prev(void)
{
   #ifdef C500_BOARD
   system("echo 1 >  /sys/class/gpio/gpio97/value"); 
   usleep(1000*120);
   system("echo 0 >  /sys/class/gpio/gpio97/value");
   #else

   system("echo 1 >  /sys/class/gpio/gpio101/value"); 
   usleep(1000*120);
   system("echo 0 >  /sys/class/gpio/gpio101/value");
   #endif
   return 0;
}

int user_bt_next(void)
{
    #ifdef C500_BOARD
   system("echo 1 >  /sys/class/gpio/gpio99/value"); 
   usleep(1000*120);
   system("echo 0 >  /sys/class/gpio/gpio99/value");
   #else
   system("echo 1 >  /sys/class/gpio/gpio102/value"); 
   usleep(1000*120);
   system("echo 0 >  /sys/class/gpio/gpio102/value");
   #endif
   return 0;
}

int user_bt_ff(void)
{
   #ifdef C500_BOARD
   system("echo 1 >  /sys/class/gpio/gpio99/value"); 
   usleep(1000*30);
   usleep(1000*120);
   system("echo 0 >  /sys/class/gpio/gpio99/value");
   #else
   system("echo 1 >  /sys/class/gpio/gpio102/value"); 
   usleep(1000*30);
   usleep(1000*120);
   system("echo 0 >  /sys/class/gpio/gpio102/value");

   #endif
   return 0;
}

int user_bt_rew(void)
{
    #ifdef C500_BOARD
   system("echo 1 >  /sys/class/gpio/gpio97/value");
   usleep(1000*30);
   usleep(1000*120);
   system("echo 0 >  /sys/class/gpio/gpio97/value");
   #else
   system("echo 1 >  /sys/class/gpio/gpio101/value");
   usleep(1000*30);
   usleep(1000*120);
   system("echo 0 >  /sys/class/gpio/gpio101/value");

   #endif
   return 0;
}

int user_bt_clear_pair(void)
{
   #ifdef C500_BOARD
   system("echo 1 >  /sys/class/gpio/gpio99/value");
   system("echo 1 >  /sys/class/gpio/gpio97/value"); 
   sleep(4);
   usleep(1000*50);
   system("echo 0 >  /sys/class/gpio/gpio99/value");
   system("echo 0 >  /sys/class/gpio/gpio97/value");
   #else

   system("echo 1 >  /sys/class/gpio/gpio102/value");
   system("echo 1 >  /sys/class/gpio/gpio101/value"); 
   sleep(4);
   usleep(1000*50);
   system("echo 0 >  /sys/class/gpio/gpio102/value");
   system("echo 0 >  /sys/class/gpio/gpio101/value");

   #endif
   return 0;
}

int user_bt_play_pause(void)
{
   #ifdef C500_BOARD
   system("echo 1 >  /sys/class/gpio/gpio98/value");
   usleep(1000*110);
   system("echo 0 >  /sys/class/gpio/gpio98/value");
   #else
   system("echo 1 >  /sys/class/gpio/gpio139/value");
   usleep(1000*110);
   system("echo 0 >  /sys/class/gpio/gpio139/value");

   #endif
   
   return 0;
}


int user_bt_shutdown(void)
{
   #ifdef C500_BOARD
   system("echo 0 >  /sys/class/gpio/gpio101/value");
   #else
   system("echo 0 >  /sys/class/gpio/gpio130/value");
   #endif
   return 0;
}


