ifeq ($(CONFIG_MTK_OVERLAY_ENGINE_SUPPORT), y)
obj-y += \
    disp_ovl_engine_api.o \
    disp_ovl_engine_core.o \
    disp_ovl_engine_dev.o \
    disp_ovl_engine_sw.o \
    disp_ovl_engine_hw.o

ccflags-y += -DMTK_OVERLAY_ENGINE_SUPPORT

ifeq ($(strip $(CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT)),y)
ccflags-y += -I$(srctree)/include -I$(srctree)/include/trustzone
ccflags-y += -DMTK_SEC_VIDEO_PATH_SUPPORT
endif
endif
