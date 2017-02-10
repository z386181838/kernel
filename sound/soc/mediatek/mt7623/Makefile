include $(srctree)/drivers/misc/mediatek/Makefile.custom

ccflags-y += -DDEBUG

# machine driver
obj-$(CONFIG_SND_SOC_MT7623) += demo-machine.o

# platform
obj-$(CONFIG_SND_SOC_MT7623) += mt7623-audio.o mt7623-memif.o mt7623-spdifout.o mt7623-hdmi-pcm.o mt7623-hdmi-raw.o mt7623-hdmi-control.o

# cpu dai
obj-$(CONFIG_SND_SOC_MT7623) += mt7623-dai.o mt7623-i2s.o 
# hardware
obj-$(CONFIG_SND_SOC_MT7623) += mt7623-afe.o

# codec driver
obj-$(CONFIG_SND_SOC_MT7623) += dummy-codec.o wm8960.o

# others
obj-$(CONFIG_SND_SOC_MT7623) += mt7623-asrc.o
