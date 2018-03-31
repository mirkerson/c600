#############################################################
#
# Build the ext4 root filesystem image
#
#############################################################

EXT4_OPTS :=

ifneq ($(strip $(BR2_TARGET_ROOTFS_EXT4_BLOCKS)),0)
EXT4_OPTS += -b $(BR2_TARGET_ROOTFS_EXT4_BLOCKS)
endif

ifneq ($(strip $(BR2_TARGET_ROOTFS_EXT4_INODES)),0)
EXT4_OPTS += -N $(BR2_TARGET_ROOTFS_EXT4_INODES)
endif

ifneq ($(strip $(BR2_TARGET_ROOTFS_EXT4_RESBLKS)),0)
EXT4_OPTS += -m $(BR2_TARGET_ROOTFS_EXT4_RESBLKS)
endif

ROOTFS_EXT4_DEPENDENCIES = host-genext2fs

define ROOTFS_EXT4_CMD
	PATH=$(TARGET_PATH) fs/ext4/genext2fs.sh -d $(TARGET_DIR) $(EXT4_OPTS) $$@ ; \
	tune2fs -j -O extents,uninit_bg,dir_index $$@ ; \
	fsck.ext4 -y $$@ ; \
	echo $(TARGETS) ; \
	echo $(TARGET_PATH)
endef

ifndef LICHEE_GEN_ROOTFS
$(eval $(call ROOTFS_TARGET,ext4))
else
ifeq ($(strip $(LICHEE_GEN_ROOTFS)), y)
$(eval $(call ROOTFS_TARGET,ext4))
endif
endif



