KERNEL_SRC=../../../../../linux-2.6.36

INC=${KERNEL_SRC}/include
MACH_INC=${KERNEL_SRC}/arch/arm/mach-sun4i/include

CC=arm-none-linux-gnueabi-gcc

CFLAGS= -I ${INC} -I ${MACH_INC}

all:sw_ctl

sw_ctl:
	${CC} ${CFLAGS} -o sw_ctl sw_ctl.c



