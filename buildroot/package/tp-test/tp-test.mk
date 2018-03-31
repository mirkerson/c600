
TP_TEST_DIR := $(BUILD_DIR)/tp-test

$(TP_TEST_DIR)/.source : 
	mkdir -pv $(TP_TEST_DIR)
	cp -rf package/tp-test/src/* $(TP_TEST_DIR)
	touch $@

$(TP_TEST_DIR)/.configured : $(TP_TEST_DIR)/.source
	touch $@


tp-test-binary: $(TP_TEST_DIR)/.configured
	echo $(TARGET_CC)
	$(MAKE) CC="$(TARGET_CC)" -C $(TP_TEST_DIR)


tp-test: tp-test-binary
	$(MAKE) DESTDIR="$(TARGET_DIR)" -C $(TP_TEST_DIR) install


##############################################################
#
# Add our target
#
#############################################################
ifeq ($(BR2_PACKAGE_TP_TEST),y)
TARGETS += tp-test
endif
