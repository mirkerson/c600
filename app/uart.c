/* 
** $Id: loadbmp.c 741 2009-03-31 07:16:18Z weiym $
**
** loadbmp.c: Sample program for MiniGUI Programming Guide
**         Load and display a bitmap.
**
** Copyright (C) 2004 ~ 2007 Feynman Software.
**
** License: GPL
*/

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

int uart_init(const char *tty)
{
	struct termios ti;
	int fd, i;
 
	fd = open(tty, O_RDWR | O_NOCTTY);
	if(fd < 0){
		printf("Can't open serial port\n");
		return -1;
	}

	tcflush(fd, TCIOFLUSH);

	if(tcgetattr(fd, &ti) < 0){
        printf("unable to get UART port setting\n");
		return -1;
	}

	cfmakeraw(&ti);

	ti.c_cflag |= CLOCAL;
	ti.c_cflag &= ~CRTSCTS;

	ti.c_lflag = 0;
	ti.c_cc[VTIME]    = 5; /* 0.5 sec */
	ti.c_cc[VMIN]     = 0;

	if(tcsetattr(fd, TCSANOW, &ti) < 0){
		printf("Can't set port settings\n");
		return -1;
	}

	/* Set initial baudrate */
	cfsetospeed(&ti, B9600);
	cfsetispeed(&ti, B9600);

	tcsetattr(fd, TCSANOW, &ti);
	tcflush(fd, TCIOFLUSH);

	return fd;
}

int main ()
{
	char buffer[16];
	int fd, len;
	
	fd = uart_init("/dev/ttyUSB0");
	if (fd < 0) {
		printf("uart open failed reboot!\n");
		return -1;
	}
	
	while(1){
		len = read(fd, buffer, sizeof(buffer));
		if (len > 0) {
			printf("uart read %s\n", buffer);
			write(fd, buffer, len);
		}
	}
	
    return 0;
}

