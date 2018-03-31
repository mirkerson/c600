/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information and source code
 *  contained herein is confidential. The software including the source code
 *  may not be copied and the information contained herein may not be used or
 *  disclosed except with the written permission of MEMSIC Inc. (C) 2009
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <unistd.h>

/* Use 'm' as magic number */
#define MXC622X_IOM			'm'

/* IOCTLs for MXC622X device */
#define MXC622X_IOC_PWRON		_IO (MXC622X_IOM, 0x00)
#define MXC622X_IOC_PWRDN		_IO (MXC622X_IOM, 0x01)
#define MXC622X_IOC_READXYZ		_IOR(MXC622X_IOM, 0x05, int[3])
#define MXC622X_IOC_READSTATUS		_IOR(MXC622X_IOM, 0x07, int[3])
#define MXC622X_IOC_SETDETECTION	_IOW(MXC622X_IOM, 0x08, unsigned char)

int main()
{
	int fd;
	int result;
	int acc[3];

	fd = open("/dev/mxc622x", O_RDWR);
	result = ioctl(fd, MXC622X_IOC_PWRON);
	sleep(1);
	for (;;) {
		printf("********************************************\n");
		result = ioctl(fd, MXC622X_IOC_READXYZ, acc);
		printf("Read result: %d\n", result);
		printf("[X - %04d] [Y - %04d] [Z - %04d]\n", 
			acc[0], acc[1], acc[2]);
		sleep(1);
	}
	close(fd);

	return 0;
}

