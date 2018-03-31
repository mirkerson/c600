#use "-Os" flags.
use-flags := -Os
#Don't use "-O2" flags.
not-use-flags := -O2
KBUILD_CFLAGS := $(filter-out $(not-use-flags), $(KBUILD_CFLAGS))
KBUILD_CFLAGS += $(use-flags)
GCOV_PROFILE := n

subdir- += standby
obj-y		+= 	suniv-pm.o \
				pm_of.o \
				standby.o \
				mem_clk.o \
				mem_int.o \
				mem_key.o \
				mem_sram.o \
				mem_divlibc.o \
				mem_ccu-sunivw1p1.o \
				mem_twi.o \
				mem_tmr.o \
				mem_gpio.o \
				pm-sunivw1.o 


$(obj)/standby.o		: $(obj)/standby/standby.code FORCE

$(obj)/standby/standby.code: FORCE
	$(Q)$(MAKE) $(build)=$(obj)/standby
