# combo driver: MT6620
# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
include $(srctree)/drivers/misc/mediatek/combo/Makefile.combo
ifneq ($(KERNELRELEASE),)
	#subdir-ccflags-y can be used in 2.6.34 in the future
	#subdir-ccflags-y += -I$(src)/common/include -I$(src)/common/linux/include
	ifneq ($(MTK_COMBO_CHIP_MT6628),)
		ifneq ($(filter y,$(CONFIG_MTK_MERGE_INTERFACE_SUPPORT)),)
			subdir-ccflags-y += -DMTK_MERGE_INTERFACE_SUPPORT -DMT6628
		endif
	endif
	obj-y += common/
	obj-y += drv_wlan/
	subdir-ccflags-y += -D$(MTK_COMBO_CHIP)
    	obj-n := dummy.o
	#skip bt hci driver
	#obj-y	+= drv_bt/

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    PWD  := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
