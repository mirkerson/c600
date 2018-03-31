#############################################################
#
# fuse-exfat
#
#############################################################

FUSE_EXFAT_VERSION = 0.9.5
FUSE_EXFAT_SOURCE = fuse-exfat-0.9.5.tar.gz
FUSE_EXFAT_SITE = http://exfat.googlecode.com/files
FUSE_EXFAT_INSTALL_STAGING = YES
FUSE_EXFAT_DEPENDENCIES = libfuse



APPS = fuse/mount.exfat-fuse dump/dumpexfat fsck/exfatfsck mkfs/mkexfatfs \
	           label/exfatlabel

define FUSE_EXFAT_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" AR="$(TARGET_AR)" -C $(@D)
endef

define FUSE_EXFAT_INSTALL_STAGING_CMDS
	cd $(@D) && cp $(APPS) $(STAGING_DIR)/sbin
endef

define FUSE_EXFAT_INSTALL_TARGET_CMDS
	cp -f $(@D)/fuse/mount.exfat-fuse $(TARGET_DIR)/sbin/mount.exfat
	cp -f $(@D)/fsck/exfatfsck $(TARGET_DIR)/sbin/fsck.exfat
	cp -f $(@D)/mkfs/mkexfatfs $(TARGET_DIR)/sbin/mkfs.exfat
	cp -f $(@D)/dump/dumpexfat $(TARGET_DIR)/sbin/dumpexfat
	cp -f $(@D)/label/exfatlabel $(TARGET_DIR)/sbin/exfatlabel
endef

$(eval $(call GENTARGETS,package,fuse-exfat))
