#############################################################
#
#pvr-utils
#
#############################################################
PVR_UTILS_DIR := $(BUILD_DIR)/pvr-utils
PVR_UTILS_SOURCE := pvr-utils.tar.gz
PVR_UTILS_TAR_OPT := -zxvf
PVR_UTILS_PACKAGE_DIR := package/pvr-utils/

pvr-utils:
	@tar -C $(TARGET_DIR) $(PVR_UTILS_TAR_OPT) $(PVR_UTILS_PACKAGE_DIR)/$(PVR_UTILS_SOURCE)
	@tar -C $(STAGING_DIR) $(PVR_UTILS_TAR_OPT) $(PVR_UTILS_PACKAGE_DIR)/$(PVR_UTILS_SOURCE)
  
ifeq ($(BR2_PACKAGE_PVR_UTILS), y)
  TARGETS += pvr-utils
endif