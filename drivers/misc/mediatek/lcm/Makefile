
include $(srctree)/drivers/misc/mediatek/Makefile.custom

CONFIG_CUSTOM_KERNEL_LCM := $(shell echo $(patsubst "%",%,$(CONFIG_CUSTOM_KERNEL_LCM)) | tr a-z A-Z)

CUSTOM_KERNEL_FLAGS := $(foreach var,$(CONFIG_CUSTOM_KERNEL_LCM), -D$(var))
ccflags-y += $(CUSTOM_KERNEL_FLAGS)

obj-$(CONFIG_MTK_FB)			+= mt65xx_lcm_list.o
obj-$(CONFIG_CM_N070ICE_DSI_VDO)	+= cm_n070ice_dsi_vdo.o
obj-$(CONFIG_CM_N070ICE_DSI_VDO_MT8135)	+= cm_n070ice_dsi_vdo_mt8135.o
obj-$(CONFIG_NT35521_WXGA_DSI_VDO_HH060IA)	+= nt35521_wxga_dsi_vdo_hh060ia.o
obj-$(CONFIG_NT51012_WXGA_DSI_VDO) += nt51012_wxga_dsi_vdo.o
obj-$(CONFIG_HSD070PFW3_8135)		+= HSD070PFW3_8135.o
obj-$(CONFIG_CPT_CLAA101FP01_DSI_VDO)		+= CPT_CLAA101FP01_DSI_VDO.o
obj-$(CONFIG_CPT_CLAP070WP03XG_SN65DSI83)	+= cpt_clap070wp03xg_sn65dsi83.o sn65dsi83_i2c.o
obj-$(CONFIG_NT35517_QHD_DSI_VIDEO)		+= nt35517_qhd_dsi_video.o
obj-$(CONFIG_CPT_CLAP070WP03XG_LVDS)		+= CPT_CLAP070WP03XG_LVDS.o
obj-$(CONFIG_NT35590_HD720_DSI_CMD_AUO)		+= nt35590_hd720_dsi_cmd_auo.o
obj-$(CONFIG_HX8392A_DSI_CMD)		+= hx8392a_dsi_cmd.o
obj-$(CONFIG_HX8392A_DSI_VDO)		+= hx8392a_dsi_vdo.o