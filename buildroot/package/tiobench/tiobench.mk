

TIOBENCH_DIR := $(BUILD_DIR)/tiobench

$(TIOBENCH_DIR)/.source : 
	mkdir -pv $(TIOBENCH_DIR)
	cp -rf package/tiobench/src/* $(TIOBENCH_DIR)
	touch $@

$(TIOBENCH_DIR)/.configured : $(TIOBENCH_DIR)/.source
	touch $@


tiobench-binary: $(TIOBENCH_DIR)/.configured
	$(MAKE) BUILD_DIR=$(BUILD_DIR) LINK="$(TARGET_CC)" CC="$(TARGET_CC)" -C $(TIOBENCH_DIR) all

tiobench: tiobench-binary
	$(MAKE) PREFIX="$(TARGET_DIR)" DESTDIR="$(TARGET_DIR)" -C $(TIOBENCH_DIR) install


##############################################################
#
# Add our target
#
#############################################################
ifeq ($(BR2_PACKAGE_TIOBENCH),y)
TARGETS += tiobench
endif
