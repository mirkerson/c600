

LCD_TEST_DIR := $(BUILD_DIR)/lcd-test

$(LCD_TEST_DIR)/.source : 
	mkdir -pv $(LCD_TEST_DIR)
	cp -rf package/lcd-test/src/* $(LCD_TEST_DIR)
	touch $@

$(LCD_TEST_DIR)/.configured : $(LCD_TEST_DIR)/.source
	touch $@


lcd-test-binary: $(LCD_TEST_DIR)/.configured
	echo $(TARGET_CC)
	$(MAKE) CC="$(TARGET_CC)" -C $(LCD_TEST_DIR)


lcd-test: lcd-test-binary
	$(MAKE) DESTDIR="$(TARGET_DIR)" -C $(LCD_TEST_DIR) install


##############################################################
#
# Add our target
#
#############################################################
ifeq ($(BR2_PACKAGE_LCD_TEST),y)
TARGETS += lcd-test
endif
