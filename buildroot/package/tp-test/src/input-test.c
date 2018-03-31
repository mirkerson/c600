#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>


#define CAPTP_DEV "/dev/input/event0" 
#define KEY_DEV "/dev/input/event1" 
#define IR_DEV "/dev/input/event1" 
#define TOUS_DEV "/dev/input/event2"
//#define TOUS_DEV "/dev/event0"
#define MOU_DEV  "/dev/input/event7"
#define TOUP_DEV "/dev/input/event7"
static int ts_fd = -1; 
static struct input_event data; 


#define ABS_X			0x00
#define ABS_Y			0x01
#define ABS_Z			0x02
#define ABS_RX			0x03
#define ABS_RY			0x04
#define ABS_RZ			0x05
#define ABS_THROTTLE		0x06
#define ABS_RUDDER		0x07
#define ABS_WHEEL		0x08
#define ABS_GAS			0x09
#define ABS_BRAKE		0x0a
#define ABS_HAT0X		0x10
#define ABS_HAT0Y		0x11
#define ABS_HAT1X		0x12
#define ABS_HAT1Y		0x13
#define ABS_HAT2X		0x14
#define ABS_HAT2Y		0x15
#define ABS_HAT3X		0x16
#define ABS_HAT3Y		0x17
#define ABS_PRESSURE		0x18
#define ABS_DISTANCE		0x19
#define ABS_TILT_X		0x1a
#define ABS_TILT_Y		0x1b
#define ABS_TOOL_WIDTH		0x1c

#define ABS_VOLUME		0x20

#define ABS_MISC		0x28

#define ABS_MT_SLOT		0x2f	/* MT slot being modified */
#define ABS_MT_TOUCH_MAJOR	0x30	/* Major axis of touching ellipse */
#define ABS_MT_TOUCH_MINOR	0x31	/* Minor axis (omit if circular) */
#define ABS_MT_WIDTH_MAJOR	0x32	/* Major axis of approaching ellipse */
#define ABS_MT_WIDTH_MINOR	0x33	/* Minor axis (omit if circular) */
#define ABS_MT_ORIENTATION	0x34	/* Ellipse orientation */
#define ABS_MT_POSITION_X	0x35	/* Center X ellipse position */
#define ABS_MT_POSITION_Y	0x36	/* Center Y ellipse position */
#define ABS_MT_TOOL_TYPE	0x37	/* Type of touching device */
#define ABS_MT_BLOB_ID		0x38	/* Group a set of packets as a blob */
#define ABS_MT_TRACKING_ID	0x39	/* Unique ID of initiated contact */
#define ABS_MT_PRESSURE		0x3a	/* Pressure on contact area */


static int init_device(char *TS_DEV) 
{ 
    if((ts_fd = open(TS_DEV, O_RDONLY)) < 0) 
    { 
        printf("Error open %s\n\n", TS_DEV); 
        return -1; 
    } 
    return ts_fd; 
} 

static int test_key() 
{ 
	printf("Begin Key Test\n");
	printf("Please Press The Key\n");
    if(init_device(KEY_DEV) < 0) 
        return -1; 
    while(1) 
    { 
        read(ts_fd, &data, sizeof(data)); 
        if (data.type == EV_KEY) 
        {
        	switch(data.code)
        	{
        		case 114:
        		{
        			if(data.value == 1)
        			{
        				printf("VOL- KEY Down\n");
        			}
        			
        			if(data.value == 2)
        			{
        				printf("VOL- KEY Repeat\n");
        			}
        			
        			if(data.value == 0)
        			{
        				printf("VOL- KEY UP\n");
        				
        			}
        			break;
        		}
        	   case 115:
         		{
        			if(data.value == 1)
        			{
        				printf("VOL+ KEY Down\n");
        			}
        			
        			if(data.value == 2)
        			{
        				printf("VOL+ KEY Repeat\n");
        			}
        			
        			if(data.value == 0)
        			{
        				printf("VOL+ KEY UP\n");
        				
        			}
        			break;
        		}
        	   case 28:
         		{
        			if(data.value == 1)
        			{
        				printf("Enter KEY Down\n");
        			}
        			
        			if(data.value == 2)
        			{
        				printf("Enter KEY Repeat\n");
        			}
        			
        			if(data.value == 0)
        			{
        				printf("Enter KEY UP\n");
        				
        			}
        			break;
        		}    
        	   case 165:
         		{
        			if(data.value == 1)
        			{
        				printf("PREV KEY Down\n");
        			}
        			
        			if(data.value == 2)
        			{
        				printf("PREV KEY Repeat\n");
        			}
        			
        			if(data.value == 0)
        			{
        				printf("PREV KEY UP\n");
        				
        			}
        			break;
        		}     	
        	   case 163:
         		{
        			if(data.value == 1)
        			{
        				printf("NEXT KEY Down\n");
        			}
        			
        			if(data.value == 2)
        			{
        				printf("NEXT KEY Repeat\n");
        			}
        			
        			if(data.value == 0)
        			{
        				printf("NEXT KEY UP\n");
        				
        			}
        			break;
        		} 
        		default :
                break;
        	}
        }
    } 
    return 0; 
} 

static int test_ir() 
{ 
	printf("Begin Ir Test\n");
	printf("Please Press The IR Key\n");
    if(init_device(IR_DEV) < 0) 
        return -1; 
    while(1) 
    { 
        read(ts_fd, &data, sizeof(data)); 
        if (data.type == EV_KEY) 
        {
        	switch(data.code)
        	{
        		case 105:
        		{
        			if(data.value == 1)
        			{
        				printf("Left KEY Down\n");
        			}
        			
        			if(data.value == 2)
        			{
        				printf("Left KEY Repeat\n");
        			}
        			
        			if(data.value == 0)
        			{
        				printf("Left KEY UP\n");
        				
        			}
        			break;
        		}
        	   case 106:
         		{
        			if(data.value == 1)
        			{
        				printf("Right KEY Down\n");
        			}
        			
        			if(data.value == 2)
        			{
        				printf("Right KEY Repeat\n");
        			}
        			
        			if(data.value == 0)
        			{
        				printf("Right KEY UP\n");
        				
        			}
        			break;
        		}
        	   case 28:
         		{
        			if(data.value == 1)
        			{
        				printf("Enter KEY Down\n");
        			}
        			
        			if(data.value == 2)
        			{
        				printf("Enter KEY Repeat\n");
        			}
        			
        			if(data.value == 0)
        			{
        				printf("Enter KEY UP\n");
        				
        			}
        			break;
        		}    
        	   case 103:
         		{
        			if(data.value == 1)
        			{
        				printf("UP KEY Down\n");
        			}
        			
        			if(data.value == 2)
        			{
        				printf("UP KEY Repeat\n");
        			}
        			
        			if(data.value == 0)
        			{
        				printf("UP KEY UP\n");
        				
        			}
        			break;
        		}     	
        	   case 108:
         		{
        			if(data.value == 1)
        			{
        				printf("Down KEY Down\n");
        			}
        			
        			if(data.value == 2)
        			{
        				printf("Down KEY Repeat\n");
        			}
        			
        			if(data.value == 0)
        			{
        				printf("Down KEY UP\n");
        				
        			}
        			break;
        		} 
        		default :
                break;
        	}
        }
    } 
    return 0; 
} 

static int test_mouse() 
{ 
    if(init_device(MOU_DEV) < 0) 
        return -1; 
    while(1) 
    { 
        read(ts_fd, &data, sizeof(data)); 
        if (data.type == EV_KEY) 
        { 
            printf(" type = EV_KEY, code = %s, value = %d\n", 
                data.code == BTN_LEFT ? "MOUSE_LEFT" : 
                data.code == BTN_RIGHT ? "MOUSE_RIGHT" : 
                data.code == BTN_MIDDLE ? "MOUSE_MIDDLE" : 
                data.code == BTN_SIDE ? "MOUSE_SIDE" : 
                "Unkonw", data.value); 
        } 
        else if(data.type == EV_REL) 
        { 
            printf(" type = EV_ABS, code = %s, value = %d\n", 
                data.code == REL_X ? "ABS_X" : 
                data.code == REL_Y ? "ABS_Y" : 
                data.code == ABS_WHEEL ? "MOUSE_WHEEL" : 
                data.code == ABS_PRESSURE ? "ABS_PRESSURE" : 
                "Unkown", data.value); 
        } 
    } 
    return 0; 
} 

static int test_touch_screen() 
{ 
	printf("Begin Touch Screen Test\n");
	printf("Please Touch The screen\n");
    if(init_device(TOUS_DEV) < 0) 
        return -1; 
    while(1) 
    { 
        read(ts_fd, &data, sizeof(data)); 
        printf("data.type == %x   \
                data.code == %x   \
                data.value == %d  \n",\
                data.type, data.code, data.value);
                
        if (data.type == EV_KEY) 
        { 
            printf(" type: EV_KEY, event = %s, value = %d\n\n", 
                data.code == BTN_TOUCH ? "BTN_TOUCH" : "Unkown", data.value); 
        } 
        else if(data.type == EV_ABS) 
        { 
            printf(" type: EV_ABS, event = %s, value = %d\n\n", 
                data.code == ABS_MT_POSITION_X ? "ABS_MT_POSITION_X" : 
                data.code == ABS_MT_POSITION_Y ? "ABS_MT_POSITION_Y" : 
                data.code == ABS_MT_TOUCH_MAJOR ? "ABS_MT_TOUCH_MAJOR" : 
                "Unkown", data.value); 
        } 
    } 
    return 0; 
} 

static int test_touch_pancel() 
{ 
    if(init_device(TOUP_DEV) < 0) 
        return -1; 
    while(1) 
    { 
        read(ts_fd, &data, sizeof(data)); 
        if (data.type == EV_KEY) 
        { 
            printf(" type = EV_KEY, code = %s, value = %d\n", 
                data.code == BTN_LEFT ? "MOUSE_LEFT" : 
                data.code == BTN_RIGHT ? "MOUSE_RIGHT" : 
                data.code == BTN_MIDDLE ? "MOUSE_MIDDLE" : 
                data.code == BTN_SIDE ? "MOUSE_SIDE" : 
                "Unkonw", data.value); 
        } 
        else if(data.type == EV_REL) 
        { 
            printf(" type = EV_ABS, code = %s, value = %d\n", 
                data.code == REL_X ? "ABS_X" : 
                data.code == REL_Y ? "ABS_Y" : 
                data.code == ABS_WHEEL ? "MOUSE_WHEEL" : 
                data.code == ABS_PRESSURE ? "ABS_PRESSURE" : 
                "Unkown", data.value); 
        } 
    } 
    return 0; 
} 

int main() 
{ 
    static int i; 
select:    printf("Please select device:\n0.KeyBoard\n1.IR\n2.TouchScreen\n"); 
    scanf("%d",&i); 
    switch(i){ 
        case 0: 
            test_key(); 
            break; 
        case 1: 
            test_ir(); 
            break; 
        case 2: 
            test_touch_screen(); 
            break; 
        default: 
            printf("Wrong device, Please select again!\n\n"); 
            break;        
    } 
    goto select; 
        return 0; 
}


