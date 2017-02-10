#
# for USB OTG silicon based on Mentor Graphics INVENTRA designs
#
ccflags-y += -DID_PIN_USE_EX_EINT
ccflags-y += -DMUSB_QMU_SUPPORT
ccflags-y += -DMTK_USB_NO_BATTERY
#ccflags-y += -DMUSB_QMU_SUPPORT_HOST
#ccflags-y += -DMUSB_QMU__HOST_BULK_RX_AUTO_SPLIT

obj-$(CONFIG_USB_MTK_HDRC) += musb_hdrc.o
obj-$(CONFIG_USB_MTK_HDRC) += $(subst ",,$(CONFIG_MTK_PLATFORM))/
musb_hdrc-y := musb_core.o

musb_hdrc-y					+= musb_gadget_ep0.o musb_gadget.o
musb_hdrc-y					+= musb_virthub.o musb_host.o
musb_hdrc-y					+= musbhsdma.o
musb_hdrc-$(CONFIG_USB_MTK_DEBUG_FS)		+= musb_debugfs.o
musb_hdrc-y  				+= mtk_qmu.o musb_qmu.o
