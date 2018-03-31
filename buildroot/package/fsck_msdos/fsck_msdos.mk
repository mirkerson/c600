FSCK_MSDOS_DIR:=$(BUILD_DIR)/fsck-msdos

$(FSCK_MSDOS_DIR)/.source : 
	mkdir -pv $(FSCK_MSDOS_DIR)  $(FSCK_MSDOS_HOST_DIR)
	cp -rf package/fsck_msdos/fsck/* $(FSCK_MSDOS_DIR)
	
	touch $@
$(FSCK_MSDOS_DIR)/.configured: $(FSCK_MSDOS_DIR)/.source
	touch $@
fsck-msdos-binary: $(FSCK_MSDOS_DIR)/.configured 
	mkdir -pv $(HOST_DIR)/usr/bin
	$(MAKE) BUILD_DIR=$(BUILD_DIR) CC="$(TARGET_CC)" -C $(FSCK_MSDOS_DIR) 
 
	
fsck-msdos: fsck-msdos-binary
	$(MAKE) DESTDIR="$(TARGET_DIR)" -C $(FSCK_MSDOS_DIR) install
	

##############################################################
#
# Add our target
#
#############################################################
ifeq ($(BR2_PACKAGE_FSCK_MSDOS),y)
TARGETS += fsck-msdos

endif

