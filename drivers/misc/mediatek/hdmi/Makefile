# drivers/video/mtk/Makefile

include $(srctree)/drivers/misc/mediatek/Makefile.custom

ifeq ($(CONFIG_MTK_INTERNAL_HDMI_SUPPORT), y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(CONFIG_MTK_PLATFORM)
endif
ifeq ($(CONFIG_MTK_INTERNAL_MHL_SUPPORT), y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/hdmi/mhltx/$(CONFIG_MTK_PLATFORM)
endif
ifeq ($(CONFIG_CUSTOM_KERNEL_HDMI),"nxp_tda19989")
	ccflags-y +=	-DHDMI_TDA19989
endif
ifeq ($(CONFIG_CUSTOM_KERNEL_HDMI),"sil9024")
	obj-y	+= sil9024/
endif

ifeq ($(CONFIG_CUSTOM_KERNEL_HDMI),"it668x")
  ccflags-y +=	-DMHL_BRIDGE_SUPPORT/
	obj-y	+= it668x/
endif

ifeq ($(CONFIG_MTK_INTERNAL_HDMI_SUPPORT), y)
obj-y  += hdmitx/$(subst ",,$(CONFIG_MTK_PLATFORM))/
endif

ifeq ($(CONFIG_MTK_INTERNAL_MHL_SUPPORT), y)
obj-y   += mhltx/$(subst ",,$(CONFIG_MTK_PLATFORM))/
endif

ifneq ($(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT),)
ccflags-y += -I$(srctree)/include/trustzone/
endif
