
include $(srctree)/drivers/misc/mediatek/Makefile.custom

ifneq ($(CONFIG_ARCH_MTK_PROJECT),)
ARCH_MTK_PROJECT := $(shell echo $(CONFIG_ARCH_MTK_PROJECT))
obj-y   += core/
#obj-y   += dct/
endif

#obj-y   +=  accelerometer/
#obj-y   +=  alsps/
#obj-y   +=  cam_cal/
#obj-y   +=  camera/
#obj-y   +=  eeprom/
#obj-y   +=  flashlight/
#obj-y   +=  gyroscope/
#obj-y   +=  headset/
#obj-y   +=  imgsensor/
#obj-y   +=  kpd/
#obj-y   +=  lcm/
obj-y   +=  leds/
#obj-y   +=  lens/
obj-$(CONFIG_MAGNETOMETER_MTK_SUPPORT)   +=  magnetometer/
#obj-y   +=  sound/
#obj-y   +=  ssw/
#obj-y   +=  vibrator/

