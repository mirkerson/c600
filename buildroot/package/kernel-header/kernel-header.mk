###############################################################################
#
# Linux kernel 2.6 target
#
###############################################################################


KERNEL_HEADER_DIR := $(BUILD_DIR)/kernel-header

$(KERNEL_HEADER_DIR)/.stamp_extracted:
	@$(call MESSAGE,"Extracting kernel header")
	mkdir -pv $(KERNEL_HEADER_DIR)
	tar -C $(KERNEL_HEADER_DIR)/ -jxf dl/linux-2.6.36-android.tar.bz2
	$(Q)touch $@

kernel-header: $(KERNEL_HEADER_DIR)/.stamp_extracted

ifeq ($(BR2_KERNEL_HEADER),y)
TARGETS+=kernel-header
endif

