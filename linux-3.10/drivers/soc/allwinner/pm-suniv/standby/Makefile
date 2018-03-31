#makefile for standby.bin
always  := standby.code
targets := standby.elf

STANDBY_XN_FILES := standby.xn

GCOV_PROFILE=no

#use "-Os" flags.
#Don't use "-O2" flags.
KBUILD_CFLAGS := -g -c -nostdlib -march=armv5te -D__LINUX_ARM_ARCH__=5 -marm -fno-unwind-tables -fno-jump-tables -fno-asynchronous-unwind-tables -mlittle-endian -O2

#Include the cur dir.
KBUILD_CPPFLAGS += -I.

LD_FLAGS = -static
LIBS =

INCLUDE   = -I. \
       -I$(KDIR)/include \
       -I$(KDIR)/drivers/soc/allwinner/standby/include \
       -I$(KDIR)/drivers/soc/allwinner/pm

standby-y := main.o suniv-standby.o dram_sys.o standby_clock.o
standby-y := $(addprefix $(obj)/,$(standby-y))

$(obj)/standby.code: $(obj)/standby.elf FORCE
	$(Q)$(CROSS_COMPILE)objcopy -O binary $(obj)/standby.elf $(obj)/standby.code
#$(call if_changed,objcopy)
	rm -rf *.o $(obj)/./*.o


$(obj)/standby.elf: $(obj)/$(STANDBY_XN_FILES) $(standby-y)
	$(Q)$(CROSS_COMPILE)ld -T $(obj)/$(STANDBY_XN_FILES) $(LD_FLAGS) $(LIBS) -EL  $(standby-y) -o $(obj)/standby.elf -Map $(obj)/standby.map
	$(Q)$(CROSS_COMPILE)objdump -D $(obj)/standby.elf > $(obj)/standby.lst

#$(call if_changed,ld)

clean-files += standby.code standby.elf
