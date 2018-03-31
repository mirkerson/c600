
mali-3d:
	tar -C $(TARGET_DIR) -xvf package/mali-3d/sun4i_mali-bin.tar
	tar -C $(STAGING_DIR) -xvf package/mali-3d/sun4i_mali-dev.tar


##############################################################
#
# Add our target
#
#############################################################
ifeq ($(BR2_PACKAGE_MALI_3D),y)
TARGETS += mali-3d
endif
