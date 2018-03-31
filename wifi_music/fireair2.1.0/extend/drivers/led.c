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

#define FILE_NAME_LEN	64


volatile static int g_led_status = LED_WIFI_DISCONNECT;

static int gpio_get_node(const char *gpio_name,unsigned node_t,char *dev_node,unsigned dev_node_len)
{
	const char *sys_fs_dir = "/sys/class/gpio_sw/";
	const char *node_arr[] = {
		"mul_sel",
		"pull",
		"drv_level",
		"data"
	};

	if(node_t < GPIO_NODE_DEFAULT) {
		return snprintf(dev_node, dev_node_len, "%s%s/%s", sys_fs_dir, gpio_name, node_arr[node_t]);
	}

	printf("[%s] [%u] get node fail\n",gpio_name,node_t);

	return ERROR;
}

static int writetoNode(const char *dev_node,const char *buffer,unsigned len)
{
	int fd;
	int ret;

	fd = open(dev_node, O_WRONLY);
	if (fd < 0) {
		printf("[w]open %s fail:(%s)\n",dev_node,strerror(errno));
		return ERROR;
	}

	ret = write(fd, buffer, len);

	if (ret < 0) {
		printf("[w]write %s fail:(%s)\n", dev_node, strerror(errno));
		ret = ERROR;
	}

	//flush(fd);

	if (close(fd) < 0) {
		printf("[w]close %s fail:(%s)\n", dev_node, strerror(errno));
	}

	return OK;
}

static int readfromNode(const char *dev_node,const char *buffer,unsigned len)
{
	int fd;
	int ret ;

	fd = open(dev_node, O_RDONLY);
	if (fd < 0) {
		printf("[r]open %s fail:(%s)\n",dev_node,strerror(errno));
		return ERROR;
	}

	ret = read(fd, (void*)buffer, len);

	if (ret < 0) {
		printf("[r]read %s fail:(%s)\n", dev_node, strerror(errno));
		ret = ERROR;
	}

	if (close(fd) < 0) {
		printf("[r]close %s fail:(%s)\n", dev_node, strerror(errno));
	}

	return OK;
}

int gpio_set_pin_io_status(const char *gpio_name,unsigned if_set_to_output_status)
{
	char dev_node[FILE_NAME_LEN] = {0};
	char value[2] = {0};

	if(gpio_get_node(gpio_name,GPIO_NODE_MUL_SEL,dev_node,sizeof(dev_node)) <= 0) {
		return ERROR;
	}

	value[0] = '0' + (if_set_to_output_status > PIN_DIR_INPUT ? 1 : 0);

	return writetoNode(dev_node,value,1);
}

int gpio_get_pin_io_status(const char *gpio_name,unsigned *p_if_output_status)
{
	char dev_node[FILE_NAME_LEN] = {0};
	char value[2] = {0};

	if(gpio_get_node(gpio_name,GPIO_NODE_MUL_SEL,dev_node,sizeof(dev_node)) <= 0) {
		return ERROR;
	}

	if (readfromNode(dev_node,&value,sizeof(value)) < 0) {
		return ERROR;
	}

	*p_if_output_status = (value[0] == '1') ? PIN_DIR_OUTPUT : PIN_DIR_INPUT;

	return OK;
}

int gpio_set_pin_pull(const char *gpio_name,unsigned set_pull_status)
{
	char dev_node[FILE_NAME_LEN] = {0};
	char value[2] = {0};

	if(set_pull_status >= PIN_PULL_DEFAULT) {
		return ERROR;
	}

	if(gpio_get_node(gpio_name,GPIO_NODE_PULL,dev_node,sizeof(dev_node)) <= 0) {
		return ERROR;
	}

	value[0] = '0' + set_pull_status;
	return writetoNode(dev_node, value, 1);
}

int gpio_get_pin_pull(const char *gpio_name,unsigned *p_pull_status)
{
	char dev_node[FILE_NAME_LEN] = {0};
	char value[2] = {0};

	if(gpio_get_node(gpio_name,GPIO_NODE_PULL,dev_node,sizeof(dev_node)) <= 0) {
		return ERROR;
	}

	if (readfromNode(dev_node,&value,sizeof(value)) < 0) {
		return ERROR;
	}

	*p_pull_status = (value[0] - '0');

	return OK;
}

int gpio_set_pin_driver_level(const char *gpio_name,unsigned set_driver_level)
{
	char dev_node[FILE_NAME_LEN] = {0};
	char value[2] = {0};

	if(set_driver_level >= PIN_MULTI_DRIVING_DEFAULT) {
		return ERROR;
	}

	if(gpio_get_node(gpio_name,GPIO_NODE_DRV_LEVEL,dev_node,sizeof(dev_node)) <= 0) {
		return ERROR;
	}

	value[0] = '0' + set_driver_level;
	return writetoNode(dev_node, value, 1);
}

int gpio_get_pin_driver_level(const char *gpio_name,unsigned *p_driver_level)
{
	char dev_node[FILE_NAME_LEN] = {0};
	char value[2] = {0};

	if(gpio_get_node(gpio_name,GPIO_NODE_DRV_LEVEL,dev_node,sizeof(dev_node)) <= 0) {
		return ERROR;
	}

	if (readfromNode(dev_node,&value,sizeof(value)) < 0) {
		return ERROR;
	}

	*p_driver_level = (value[0] - '0');

	return OK;
}

int gpio_write_pin_value(const char *gpio_name,unsigned value_to_gpio)
{
	char dev_node[FILE_NAME_LEN] = {0};
	char value[2] = {0};

	if(gpio_get_node(gpio_name,GPIO_NODE_DATA,dev_node,sizeof(dev_node)) <= 0) {
		return ERROR;
	}

	value[0] = '0' + (value_to_gpio > PIN_DATA_LOW ? 1 : 0);

	return writetoNode(dev_node, value, 1);
}

int gpio_read_pin_value(const char *gpio_name,unsigned *p_value)
{
	char dev_node[FILE_NAME_LEN] = {0};
	char value[2] = {0};

	if(gpio_get_node(gpio_name,GPIO_NODE_DATA,dev_node,sizeof(dev_node)) <= 0) {
		return ERROR;
	}

	if (readfromNode(dev_node,&value,sizeof(value)) != OK) {
		return ERROR;
	}

	*p_value = (value[0] == '1') ? PIN_DATA_HIGH : PIN_DATA_LOW;

	return OK;
}



//bi color led
int biled_onoff(const char* gpio_name, int value, const char* gpio2_name, int value2)
{
    if(OK != gpio_set_pin_io_status(gpio_name, PIN_DIR_OUTPUT)) {
        return ERROR;
    }

    if(OK != gpio_write_pin_value(gpio_name,value)) {
        return ERROR;
    }

    if(OK != gpio_set_pin_io_status(gpio2_name, PIN_DIR_OUTPUT)) {
        return ERROR;
    }

    if(OK != gpio_write_pin_value(gpio2_name,value2)) {
        return ERROR;
    }

    return OK;
}

int led_onoff(const char* gpio_name,int value)
{
    if(OK != gpio_set_pin_io_status(gpio_name, PIN_DIR_OUTPUT)) {
        return ERROR;
    }

    if(OK != gpio_write_pin_value(gpio_name,value)) {
        return ERROR;
    }

    return OK;
}

int user_red_on(void)
{
     system("echo 0 >  /sys/class/gpio/gpio113/value");
}

int user_green_on(void)
{
     system("echo 1 >  /sys/class/gpio/gpio133/value");
}

int user_red_off(void)
{
     system("echo 1>  /sys/class/gpio/gpio113/value");
}

int user_green_off(void)
{
     system("echo 0 >  /sys/class/gpio/gpio133/value");
}


int user_power_on(void)
{
    user_red_off();
}

int user_wifi_disconnect(void)
{
     user_green_on();
	 usleep(1000*800);
	 user_green_off();
	 usleep(1000*800);
}

int user_wifi_connecting(void)
{
     user_green_on();
	 usleep(1000*200);
	 user_green_off();
	 usleep(1000*200);
}

int user_wifi_connected(void)
{
     user_green_on();
}

int user_get_led_status(void)
{

	return g_led_status;

}

int user_set_led_status(int status)
{
	g_led_status = status;

}

void * user_led_thread(void *arg)
{
    int ret;
	
    while(1){
		
         ret = user_get_led_status();
         switch (ret)
         {
		 	case LED_WIFI_CONNECTED :
			
				{
                    user_wifi_connected();
					sleep(2);

				}break;
			case LED_WIFI_CONNECTING :
				{
                    user_wifi_connecting();

				}break;
			case LED_WIFI_DISCONNECT :
				{
                    user_wifi_disconnect();

				}
				break;
			default:
                printf("invalid led status\n");
				break;
         }


	}

}


int user_led_ctl(void)
{
    int ret;
	pthread_t  tid_led = 0;

    ret = pthread_create(&tid_led, NULL, &user_led_thread,NULL);  
	if(ret != 0) {  
		printf("pthread_create led failed\n");	
		return -1;
	}  
    return 0;
}

