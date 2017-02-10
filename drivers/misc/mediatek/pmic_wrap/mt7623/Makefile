include $(srctree)/drivers/misc/mediatek/Makefile.custom

obj-y +=  pwrap_hal.o

ifeq ($(CONFIG_MTK_ALPS_BOX_SUPPORT), y)
ccflags-y += -DMTK_ALPS_BOX_SUPPORT
endif
