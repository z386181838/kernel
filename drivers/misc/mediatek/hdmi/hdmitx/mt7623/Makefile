include $(srctree)/drivers/misc/mediatek/Makefile.custom

ifeq ($(CONFIG_MTK_INTERNAL_HDMI_SUPPORT), y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(MTK_PLATFORM)/inc \
			 -I$(srctree)/drivers/misc/mediatek/hdmi/inc \
             -I$(srctree)/drivers/misc/mediatek/hdmirx/$(MTK_PLATFORM)	\
             -I$(srctree)/drivers/misc/mediatek/videoif/$(MTK_PLATFORM)	\
             -I$(srctree)/drivers/misc/mediatek/dgi/$(MTK_PLATFORM) 
             

ifeq ($(CONFIG_MTK_INTERNAL_MHL_SUPPORT), y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/hdmi/mhltx/$(CONFIG_MTK_PLATFORM)
endif

ifeq ($(CONFIG_CUSTOM_KERNEL_HDMI),"nxp_tda19989")
	ccflags-y +=	-DHDMI_TDA19989
endif

ifeq ($(CONFIG_CUSTOM_KERNEL_HDMI),"it668x")
  ccflags-y +=	-DMHL_BRIDGE_SUPPORT/
endif

ifeq ($(CONFIG_MTK_OVERLAY_ENGINE_SUPPORT), y)
ccflags-y += -DMTK_OVERLAY_ENGINE_SUPPORT
endif

ifneq ($(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT),)
ccflags-y += -I$(srctree)/include/trustzone/
endif

obj-y := hdmi_drv.o hdmi_ctrl.o hdmiavd.o hdmicec.o hdmiddc.o hdmiedid.o hdmihdcp.o hdmicmd.o hdmictrl.o hdmitx.o debug.o  hdmitx_test.o

ifeq ($(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT),y)
ccflags-y += -I$(srctree)/include/trustzone/
obj-y += hdmi_ca.o
endif


endif
