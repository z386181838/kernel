#
# for USB OTG silicon based on Mentor Graphics INVENTRA designs
#
ccflags-y += -DID_PIN_USE_EX_EINT
ccflags-y += -DMUSB_QMU_SUPPORT
ccflags-y += -DMTK_USB_NO_BATTERY
#ccflags-y += -DMUSB_QMU_SUPPORT_HOST

include $(srctree)/drivers/misc/mediatek/Makefile.custom
ccflags-y += -Werror

obj-$(CONFIG_USB_MTK_HDRC)	:= usb20.o usb20_phy.o
obj-$(CONFIG_USB_MTK_HDRC)  += usb20_host.o
obj-$(CONFIG_USB_MTK_OTG)   += usb20_otg_if.o
obj-$(CONFIG_USB_MTK_DEBUG_FS)	+= usb20_phy_debugfs.o
ifeq ($(CONFIG_MTK_AEE_KDUMP),y)
obj-$(CONFIG_USB_MTK_HDRC) += usb_dump.o
endif
