ifeq ($(CONFIG_MTK_WLAN_SUPPORT), y)

# Enable warning to error in all MTK subdir.
subdir-ccflags-y += -Werror

SUPPORT_MT6620 := no
SUPPORT_MT6628 := no
SUPPORT_MT6632 := no

ifneq ($(MTK_COMBO_CHIP_MT6620),)
SUPPORT_MT6620 := yes
endif

ifneq ($(MTK_COMBO_CHIP_MT6628),)
SUPPORT_MT6628 := yes
endif

ifneq ($(MTK_COMBO_CHIP_MT6632),)
SUPPORT_MT6632 := yes
endif

ifeq ($(SUPPORT_MT6620), yes)
    obj-$(CONFIG_MTK_COMBO_WIFI) += mt6620/
endif

ifeq ($(SUPPORT_MT6628), yes)
    obj-$(CONFIG_MTK_COMBO_WIFI) += mt6628/
endif

ifeq ($(SUPPORT_MT6632), yes)
    obj-$(CONFIG_MTK_COMBO_WIFI) += MT6632_WiFi_Driver_Dev/
endif

endif