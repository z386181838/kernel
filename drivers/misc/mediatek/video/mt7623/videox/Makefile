
include $(srctree)/drivers/misc/mediatek/Makefile.custom


obj-$(CONFIG_MTK_FB) += disp_drv.o                  \
		mtkfb_console.o                            	\
		disp_assert_layer.o                       	\
		mtkfb.o                                   	\
		mtkfb_vsync.o								\
		mtkfb_pattern.o                            	\
		debug.o                                    	\
		mtkfb_proc.o								\
		disp_drv_dbi.o								\
		disp_drv_dpi.o								\
		disp_drv_dsi.o								\
		disp_hal.o									\
		disp_intr.o

obj-$(CONFIG_MTK_OVERLAY_ENGINE_SUPPORT) += ovl_engine/

			 
ifeq ($(CONFIG_MTK_DITHERING_SUPPORT), y)
ccflags-y += -DDITHERING_SUPPORT
endif

ifeq ($(CONFIG_MTK_GPU_SUPPORT), y)
ccflags-y += -DHWGPU_SUPPORT
endif

ifeq ($(CONFIG_MTK_FB_UMP),y)
obj-m += disp_ump.o
endif