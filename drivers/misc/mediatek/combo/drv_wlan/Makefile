SUPPORT_MT6620 := no
SUPPORT_MT6628 := no
SUPPORT_MT6630 := no

SUPPORT_MT6620 := $(if $(filter "MT6620", $(MTK_COMBO_CHIP)),yes,no)
SUPPORT_MT6628 := $(if $(filter "MT6628", $(MTK_COMBO_CHIP)),yes,no)
SUPPORT_MT6630 := $(if $(filter "MT6630", $(MTK_COMBO_CHIP)),yes,no)

ifeq ($(SUPPORT_MT6620), yes)
    obj-y += mt6620/wlan/
    $(warning include mt6620/wlan/)
endif

ifeq ($(SUPPORT_MT6628), yes)
    obj-y += mt6628/wlan/
    $(warning include mt6628/wlan/)
endif

ifeq ($(SUPPORT_MT6630), yes)
    obj-y += mt6630/wlan/
    $(warning include mt6630/wlan/)
endif
