include $(srctree)/drivers/misc/mediatek/Makefile.custom

#add kernel source code path as head file seach path
ccflags-y += -I$(srctree)
ccflags-y += -D MTK_SDIOAUTOK_SUPPORT

obj-$(CONFIG_MTK_MMC) += dbg.o
obj-$(CONFIG_MTK_MMC) += mt_sd_misc.o sd.o
obj-$(CONFIG_MTK_MMC) += sdio_autok.o sdio_autok_proc.o

# kdump/kpainc interface move to lk 
ifeq ($(CONFIG_MTK_AEE_IPANIC),y)
obj-$(CONFIG_MTK_MMC) += mt_dump.o
endif

ifeq ($(CONFIG_MTK_EMMC_SUPPORT_OTP),yes)
obj-$(CONFIG_MTK_MMC) += emmc_otp.o
endif

ifeq ($(CONFIG_MET_MMC_PROFILE),y)
ccflags-y += -DFEATURE_MET_MMC_INDEX
endif
