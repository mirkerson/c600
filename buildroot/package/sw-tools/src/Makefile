
KDIR = $(BUILD_DIR)/kernel-header/linux-2.6.36-android
CFLAG = -static -O2 -I$(KDIR)/include -I$(KDIR)/arch/arm/mach-sun4i/include

.PHONY: all install
all:
	echo $(BUILD_DIR)
	$(CC) $(CFLAG) -static -O2 -o td td.c
	$(CC) $(CFLAG) -static -O2 -o sw_script sw_script.c
	gcc  -O2 -o sw_host_script sw_script.c
	$(CC) $(CFLAG) -static -O2 -o sw_mem sw_mem.c
	$(CC) $(CFLAG) -static -O2 -o mxc622x_test mxc622x_test.c

clear:
	rm -rf *.o

install:
	cp -f td sw_mem sw_script mxc622x_test run-test1.sh $(DESTDIR)/bin
	cp sw_host_script $(HOSTDIR)/

.PHONY: clean
clean: 
	rm -rf *.o


