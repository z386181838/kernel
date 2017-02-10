
include $(srctree)/drivers/misc/mediatek/Makefile.custom
include $(srctree)/drivers/misc/mediatek/combo/Makefile.combo

MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
MTK_FM_RX_AUDIO_FLAG := $(addprefix -D, $(shell echo $(CONFIG_MTK_FM_RX_AUDIO) | tr a-z A-Z))
subdir-ccflags-y += $(MTK_FM_RX_AUDIO_FLAG)



obj-y   += aee/
obj-y   += boot/
obj-$(CONFIG_MTK_BTIF)   	+= btif/
obj-$(CONFIG_ARCH_MT7623)   += dum-char/
obj-$(CONFIG_MTK_EMMC_SUPPORT) += mtk-emmc/
obj-y   += eint/
obj-y   += gcpu/
#obj-y   += gdma/
obj-$(CONFIG_MTK_LEGACY_GPIO_DRIVER)    += gpio/
obj-$(CONFIG_ARCH_MT7623)   += keypad/
obj-y   += lcm/
obj-y   += m4u/
obj-$(CONFIG_ZRAM) += mlog/
obj-y   += mmp/
obj-$(CONFIG_MTKPASR)       += mtkpasr/
obj-y   += mtprof/
obj-$(CONFIG_USB_MU3D_DRV)  += mu3d/
obj-$(CONFIG_MU3_PHY)       += mu3phy/
obj-y   += pa/
obj-y   += pmic_wrap/
obj-$(CONFIG_ARCH_MT7623)   += pmt/
obj-$(CONFIG_ARCH_MT7623)   += power_gs/
obj-y   += sched/
obj-y   += selinux_warning/
obj-$(CONFIG_MTK_TOUCHPANEL)+= touchpanel/
obj-y   += uart/

obj-$(CONFIG_MTK_HDMI_SUPPORT) += hdmi/
obj-$(CONFIG_MTK_FB) += video/

obj-$(CONFIG_MTK_XHCI) += xhci/
obj-y   += wdk/
obj-y	+= mmp/
obj-y	+= ram_console/
obj-y   += thermal/
obj-y   += xlog/
obj-y   += wdk/
obj-y   += sound/
obj-y   += evdo_support/
obj-y   += mrdump/
obj-y   +=  i2c/

ifneq ($(CONFIG_MTK_FPGA),y)
obj-y   += auxadc/
# gator-driver is not custom-release and will be removed
obj-y   += $(call all-subdirs-with-makefile,gator-driver)
obj-y   += ccci/
obj-y   += ccmni/
obj-y   += dbg_dump/
obj-y   += dual_ccci/
obj-y   += eccci/
obj-y   += emd_ctl/
obj-$(CONFIG_MTK_ETM) += etm/
obj-y   += freqhopping/
obj-y   += hwmon/
obj-y   += leds/
obj-$(CONFIG_MTK_NFC_SUPPORT)   += nfc/
obj-y   += pwm/
obj-y   += rtc/
obj-$(CONFIG_MTK_FB)   += sync/
#MTK in-house tee, need to be prior to kernel for fiq_smp_call support
obj-$(CONFIG_MTK_IN_HOUSE_TEE_SUPPORT) += trustzone/
obj-y	+= kernel/

ifneq ($(CONFIG_MTK_COMBO),)
	subdir-ccflags-y += -DMTK_WCN_WMT_STP_EXP_SYMBOL_ABSTRACT
	subdir-ccflags-y += -DMTK_WCN_REMOVE_KERNEL_MODULE

	BUILD_COMBO := no
	BUILD_SOC := no

	ifneq ($(filter "MT6620E3",$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_COMBO := yes
		subdir-ccflags-y += -DMTK_WCN_COMBO_CHIP_SUPPORT
		subdir-ccflags-y += -DMT6620
	endif
	ifneq ($(filter "MT6628",$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_COMBO := yes
		subdir-ccflags-y += -DMTK_WCN_COMBO_CHIP_SUPPORT
		subdir-ccflags-y += -DMT6628
	endif
	ifneq ($(filter "MT6630",$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_COMBO := yes
		subdir-ccflags-y += -DMTK_WCN_COMBO_CHIP_SUPPORT
		subdir-ccflags-y += -DMT6630
	endif
	ifeq ($(BUILD_COMBO), yes)
		obj-y   +=  combo/
	endif

	ifneq ($(filter "CONSYS_6572",$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_SOC := yes
	endif
	ifneq ($(filter "CONSYS_6582",$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_SOC := yes
	endif
	ifneq ($(filter "CONSYS_6582",$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_SOC := yes
	endif
	ifneq ($(filter "CONSYS_6592",$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_SOC := yes
	endif
	ifneq ($(filter "CONSYS_8127",$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_SOC := yes
	endif
	ifneq ($(filter "CONSYS_7623",$(CONFIG_MTK_COMBO_CHIP)),)
		BUILD_SOC := yes
	endif
	ifeq ($(BUILD_SOC), yes)
		obj-y   +=  conn_soc/
	endif
	obj-y   +=  common_detect/
endif
endif

ifneq ($(CONFIG_MTK_PLATFORM),)
obj-$(CONFIG_HAVE_CMMB_FEATURE)   += cmmbdrv/$(MTK_PLATFORM)/
obj-y   += devapc/$(MTK_PLATFORM)/
obj-y   += devinfo/$(MTK_PLATFORM)/
obj-$(CONFIG_MTK_LDVT) += ldvt/$(MTK_PLATFORM)/
obj-y   += mmc-host/$(MTK_PLATFORM)/
obj-$(CONFIG_MTK_MTD_NAND)	+= nand/
obj-y   += power/
obj-y   += smi/$(MTK_PLATFORM)/
obj-$(CONFIG_MTK_SPI)   += spi/$(MTK_PLATFORM)/
obj-y   += wdt/$(MTK_PLATFORM)/
endif

ifeq ($(CONFIG_CUSTOM_KERNEL_FLASHLIGHT),"dummy_flashlight")
obj-y   += flashlight/dummy_flashlight/
endif
ifeq ($(CONFIG_CUSTOM_KERNEL_FLASHLIGHT),"constant_flashlight")
obj-y   += flashlight/constant_flashlight/
endif
