
sun4i_cedar:
	@echo "Installing SUN4I CEDAR package"
	tar -C $(TARGET_DIR) -xf package/cedar/sun4i_cedar-bin.tar
	tar -C $(STAGING_DIR) -xf package/cedar/sun4i_cedar-dev.tar

sun3i_cedar:
	@echo "Installing SUN3I CEDAR package"
	tar -C $(TARGET_DIR) -xvf package/cedar/sun3i_cedar-bin.tar
	tar -C $(STAGING_DIR) -xvf package/cedar/sun3i_cedar-dev.tar

##############################################################
#
# Add our target
#
#############################################################
ifeq ($(BR2_PACKAGE_CEDAR), y)

ifeq ($(CONFIG_CHIP_ID), 1120)
TARGETS += sun3i_cedar
else
TARGETS += sun4i_cedar
endif

endif
