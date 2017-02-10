
include $(srctree)/drivers/misc/mediatek/Makefile.custom

obj-y += ddp_drv.o
obj-y += ddp_ovl.o
obj-y += ddp_bls.o
obj-y += ddp_color.o
obj-y += ddp_rdma.o
obj-y += ddp_wdma.o
obj-y += ddp_path.o
obj-y += ddp_debug.o
obj-y += ddp_cmdq.o
obj-y += ddp_cmdq_debug.o
obj-y += ddp_cmdq_sec.o
obj-y += ddp_dpfd.o
obj-y += ddp_aal.o
obj-y += data_rgb888_64x64.o
obj-y += data_rgb888_64x64_golden.o
obj-y += lcd_drv.o
obj-y += dpi_drv.o
obj-y += dsi_drv.o
#obj-y += dpi1_drv.o

ifeq ($(CONFIG_MTK_INTERNAL_HDMI_SUPPORT), y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(CONFIG_MTK_PLATFORM)
endif

ifeq ($(CONFIG_MTK_INTERNAL_MHL_SUPPORT), y)
ccflags-y += -I$(srctree)/drivers/misc/mediatek/hdmi/mhltx/$(CONFIG_MTK_PLATFORM)
endif

ifeq ($(CONFIG_MTK_OVERLAY_ENGINE_SUPPORT), y)
ccflags-y += -DMTK_OVERLAY_ENGINE_SUPPORT
endif


