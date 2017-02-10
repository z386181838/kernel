# drivers/video/mtk/Makefile
#
# Makefile for MT7623 video driver.
#
include $(srctree)/drivers/misc/mediatek/Makefile.custom

ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/hdmirx/  \
             -I$(MTK_PATH_SOURCE)/drivers/hdmirx/    \
             -I$(MTK_PATH_PLATFORM)/drivers/hdmirx/  \
             -I$(MTK_PATH_SOURCE)/drivers/hdmirx/    \
             -I$(srctree)/drivers/misc/mediatek/hdmi/hdmitx/$(CONFIG_MTK_PLATFORM)   \
             -I$(srctree)/drivers/misc/mediatek/videoin/$(CONFIG_MTK_PLATFORM)   \
             -I$(srctree)/drivers/misc/mediatek/videoif/$(CONFIG_MTK_PLATFORM)   \
             -I$(srctree)/drivers/misc/mediatek/dgi/$(CONFIG_MTK_PLATFORM)
                
obj-y	:= hdmirx.o mt_hdmirx_debug.o hdmi_rx_ctrl.o hdmi_rx_task.o hdmi_rx_dvi.o hdmi_rx_hal.o edid_data.o edid_eeprom.o hal_io.o vga_table.o rx_io.o hdmi_rx_cmd.o




