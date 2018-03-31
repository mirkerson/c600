#############################################################
#
# ntfs-3g
#
#############################################################

NTFS_3G_VERSION = 2011.4.12
NTFS_3G_SOURCE = ntfs-3g-$(NTFS_3G_VERSION).tgz
NTFS_3G_SITE = http://tuxera.com/opensource
NTFS_3G_CONF_OPT = --disable-ldconfig --program-prefix=""
NTFS_3G_INSTALL_STAGING = YES

$(eval $(call AUTOTARGETS,package,ntfs-3g))
