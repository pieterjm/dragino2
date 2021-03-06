# 
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: $

include $(TOPDIR)/rules.mk

PKG_NAME:=dahdi-tools
PKG_VERSION:=2.1.0.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://downloads.digium.com/pub/telephony/dahdi-tools/releases/
PKG_MD5SUM:=2e0c2866112932e54aacf3dc62f548ca

include $(INCLUDE_DIR)/package.mk



define Package/dahdi-cfg/Default
	SECTION:=net
	CATEGORY:=Network
	DEPENDS=+kmod-dahdi-linux
	URL:=http://ftp.digium.com/pub/telephony/dahdi-tools/releases
endef

define Package/dahdi-cfg
	$(call Package/dahdi-cfg/Default)
	TITLE:=DAHDI tools dahdi_cfg and dahdi_scan
endef

define Package/dahdi-monitor
	$(call Package/dahdi-cfg/Default)
	TITLE:=DAHDI tools dahdi_monitor, dahdi_speed and dahdi_test
endef

define Package/dahdi-tools-libtonezone
	$(call Package/dahdi-cfg/Default)
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=DAHDI tonezone library.
endef

#define Build/Configure
#endef

TARGET_CFLAGS += $(FPIC)
EXTRA_CFLAGS:= $(TARGET_CPPFLAGS)

# dahdi_tool build not included => need libnewt
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		HOSTCC="$(HOSTCC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		CFLAGS="$(TARGET_CFLAGS) -I. -DBUILDING_TONEZONE -I$(STAGING_DIR)/usr/include" \
		KSRC="$(LINUX_DIR)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		DAHDI_INCLUDE="$(STAGING_DIR)/usr/include" \
		CONFIGURE_SILENT="--silent" \
		dahdi_cfg dahdi_monitor dahdi_scan dahdi_speed dahdi_test fxotune libs
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include/dahdi
	$(CP) $(PKG_BUILD_DIR)/libtonezone.so $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libtonezone.a $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/tonezone.h $(1)/usr/include/dahdi/
endef

define Package/dahdi-cfg/conffiles
/etc/dahdi/system.conf
endef

define Package/dahdi-cfg/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/dahdi
	$(CP) ./files/system.conf $(1)/etc/dahdi/

	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/dahdi_cfg $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/dahdi_scan $(1)/usr/sbin/
endef

define Package/dahdi-monitor/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/dahdi_monitor $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/dahdi_speed $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/dahdi_test $(1)/usr/sbin/
endef

define Package/dahdi-tools-libtonezone/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libtonezone.so $(1)/usr/lib/
	cd $(1)/usr/lib/; ln -s libtonezone.so libtonezone.so.2.0
endef

$(eval $(call BuildPackage,dahdi-cfg))
$(eval $(call BuildPackage,dahdi-monitor))
$(eval $(call BuildPackage,dahdi-tools-libtonezone))
