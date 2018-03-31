
mali-3d-r2p3:
	tar -C $(TARGET_DIR) -xvf package/mali-3d-r2p3/sun4i_mali-bin.tar
	tar -C $(STAGING_DIR) -xvf package/mali-3d-r2p3/sun4i_mali-dev.tar


##############################################################
#
# Add our target
#
#############################################################
ifeq ($(BR2_PACKAGE_MALI_3D_R2P3),y)
TARGETS += mali-3d-r2p3
endif
