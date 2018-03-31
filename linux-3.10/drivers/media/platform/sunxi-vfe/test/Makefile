CC:= ../../../../../../out/sunivw1p1/linux/common/buildroot/external-toolchain/bin/arm-none-linux-gnueabi-gcc
#CC:=arm-none-linux-gnueabi-gcc
#LINUX_DIR = $(shell pwd)/../../../../../include/:$(shell pwd)/../../../../../arch/arm64/include
#ARCH_DIR=../../../../../arch/arm64/include/
CFLAGS := -I $(LINUX_DIR)
TARGET := csi_test

.PHONY: all clean

all: $(TARGET) 

csi_test: csi_test.c 
	$(CC) $(CFLAGS) -static  $^  -o  $@ -ljpeg -L/home/zhangchaoxian/work/lichee/linux-3.10/drivers/media/platform/sunxi-vfe/test/libjpeg/lib/ -lrt

clean:
	rm -rf $(TARGET)  
