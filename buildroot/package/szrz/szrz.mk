#SZRZ_VERSION	= 3.48
#SZRZ_SOURCE		= rzsz-3.48.tar.gz
#SZRZ_SITE		= http://freeware.sgi.com/source/rzsz/rzsz-3.48.tar.gz
#SZRZ_INSTALL_STAGING	= YES
#SZRZ_INSTALL_TARGET		= YES
#
#$(eval $(call AUTOTARGETS,package,szrz))
#
#ifeq ($(BR2_PACKAGE_SZ_RZ),y)
#TARGETS += szrz
#endif


SZRZ_DIR	 := $(BUILD_DIR)/rzsz-3.48
SZRZ_TAR_DIR := $(BUILD_DIR)/src
SZRZ_SOURCE	 := ${DL_DIR}/rzsz-3.48.tar.gz

$(SZRZ_DIR)/.source : 
	tar -zxf ${SZRZ_SOURCE} -C ${BUILD_DIR}
	mv ${SZRZ_TAR_DIR} ${SZRZ_DIR}
	touch $@

$(SZRZ_DIR)/.configured : $(SZRZ_DIR)/.source
	touch $@


szrz-binary : $(SZRZ_DIR)/.configured
	$(MAKE) BUILD_DIR=$(BUILD_DIR) CC="$(TARGET_CC)" -C $(SZRZ_DIR) posix 


szrz : szrz-binary
	cp ${SZRZ_DIR}/sz  ${TARGET_DIR}/usr/bin
	cp ${SZRZ_DIR}/rz  ${TARGET_DIR}/usr/bin

##############################################################
#
# Add our target
#
#############################################################
ifeq ($(BR2_PACKAGE_SZ_RZ),y)
TARGETS += szrz
endif
