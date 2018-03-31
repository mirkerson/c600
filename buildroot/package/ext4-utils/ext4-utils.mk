

EXT4_UTILS_DIR := $(BUILD_DIR)/ext4-utils
EXT4_UTILS_HOST_DIR := $(BUILD_DIR)/host-ext4-utils

$(EXT4_UTILS_DIR)/.source : 
	mkdir -pv $(EXT4_UTILS_DIR) $(EXT4_UTILS_HOST_DIR)
	cp -rf package/ext4-utils/src/* $(EXT4_UTILS_DIR)
	cp -rf package/ext4-utils/src/* $(EXT4_UTILS_HOST_DIR)
	touch $@

$(EXT4_UTILS_DIR)/.configured : $(EXT4_UTILS_DIR)/.source
	touch $@


ext4-utils-binary: $(EXT4_UTILS_DIR)/.configured zlib
	$(MAKE) BUILD_DIR=$(BUILD_DIR) CC="$(TARGET_CC)" -C $(EXT4_UTILS_DIR) all
	$(MAKE) BUILD_DIR=$(BUILD_DIR) CC="$(HOSTCC)" -C $(EXT4_UTILS_HOST_DIR) all
	mkdir -pv $(HOST_DIR)/usr/bin


ext4-utils: ext4-utils-binary
	$(MAKE) DESTDIR="$(TARGET_DIR)" -C $(EXT4_UTILS_DIR) install
	$(MAKE) DESTDIR="$(HOST_DIR)" -C $(EXT4_UTILS_HOST_DIR) install


##############################################################
#
# Add our target
#
#############################################################
ifeq ($(BR2_PACKAGE_EXT4_UTILS),y)
TARGETS += ext4-utils
HOSTS += ext4-utils
endif
