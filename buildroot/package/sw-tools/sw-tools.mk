

SW_TOOLS_DIR := $(BUILD_DIR)/sw-tools

$(SW_TOOLS_DIR)/.source : 
	mkdir -pv $(SW_TOOLS_DIR)
	cp -rf package/sw-tools/src/* $(SW_TOOLS_DIR)
	touch $@

$(SW_TOOLS_DIR)/.configured : $(SW_TOOLS_DIR)/.source
	touch $@


sw-tools-binary: $(SW_TOOLS_DIR)/.configured
	echo $(TARGET_CC)
	$(MAKE) BUILD_DIR=$(BUILD_DIR) CC="$(TARGET_CC)" -C $(SW_TOOLS_DIR)


sw-tools: sw-tools-binary
	$(MAKE) HOSTDIR="$(HOST_DIR)" DESTDIR="$(TARGET_DIR)" -C $(SW_TOOLS_DIR) install


##############################################################
#
# Add our target
#
#############################################################
ifeq ($(BR2_PACKAGE_SW_TOOLS),y)
TARGETS += sw-tools
endif
