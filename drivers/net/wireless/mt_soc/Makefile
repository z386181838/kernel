ifeq ($(CONFIG_MTK_WLAN_SUPPORT), y)

# Enable warning to error in all MTK subdir.
#subdir-ccflags-y += -Werror

SUPPORT_CONNSYS:= no

$(info 'choose WLAN CHIP=$(CONFIG_MTK_WLAN_CHIP)')

$(info "check_connsys")
SUPPORT_CONNSYS := $(if $(findstring CONSYS,$(CONFIG_MTK_WLAN_CHIP)),yes,no)
$(info "check_connsys_end $(SUPPORT_CONNSYS)")

ifeq ($(SUPPORT_CONNSYS), yes)
	obj-$(CONFIG_MTK_CONNSYS_WIFI) += conn_soc/
    $(info "include conn_soc")
endif

endif
