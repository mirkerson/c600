#############################################################
#
# usb_modeswitch
#
#############################################################

#USB_MODESWITCH_VERSION = 1.1.2
#USB_MODESWITCH_SOURCE = usb-modeswitch-$(USB_MODESWITCH_VERSION).tar.bz2
#USB_MODESWITCH_SITE = http://www.draisberghof.de/usb_modeswitch
#USB_MODESWITCH_DEPENDENCIES = libusb-compat

#define USB_MODESWITCH_BUILD_CMDS
#	$(TARGET_CONFIGURE_OPTS) $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
#endef

#define USB_MODESWITCH_INSTALL_TARGET_CMDS
#	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR=$(TARGET_DIR) install
#	$(INSTALL) -D $(@D)/usb_modeswitch.setup -m 0644 \
#		$(TARGET_DIR)/etc/usb_modeswitch.setup
#endef

#define USB_MODESWITCH_CLEAN_CMDS
#	rm -f $(TARGET_DIR)/usr/sbin/usb_modeswitch
#	rm -f $(TARGET_DIR)/etc/usb_modeswitch.setup
#	rm -f $(TARGET_DIR)/usr/share/man/man1/usb_modeswitch.1
#endef

#$(eval $(call GENTARGETS,package,usb_modeswitch))

USB_MODESWITCH_VERSION = 1.1.9
USB_MODESWITCH_SOURCE = usb-modeswitch-1.1.9.tar.gz
USB_MODESWITCH_SITE = http://www.draisberghof.de/usb_modeswitch
USB_MODESWITCH_DEPENDENCIES = libusb

USB_MODESWITCH_DIR-1.1.9:=$(BUILD_DIR)/usb_modeswitch-1.1.9
define USB_MODESWITCH_BUILD_CMDS		
	$(MAKE) BUILD_DIR=$(BUILD_DIR) CC="$(TARGET_CC)" -C $(USB_MODESWITCH_DIR-1.1.9) 
endef
define USB_MODESWITCH_INSTALL_TARGET_CMDS	
	$(MAKE) DESTDIR="$(TARGET_DIR)" -C $(USB_MODESWITCH_DIR-1.1.9) install
endef
$(eval $(call GENTARGETS,package,usb_modeswitch))