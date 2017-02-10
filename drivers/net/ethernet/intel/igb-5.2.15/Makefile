################################################################################
#
# Intel(R) Gigabit Ethernet Linux driver
# Copyright(c) 2007-2014 Intel Corporation.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU General Public License,
# version 2, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# The full GNU General Public License is included in this distribution in
# the file called "COPYING".
#
# Contact Information:
# Linux NICS <linux.nics@intel.com>
# e1000-devel Mailing List <e1000-devel@lists.sourceforge.net>
# Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
#
################################################################################

###########################################################################
obj-$(CONFIG_IGB_V5215) += igb_v5215.o
# Driver files
igb_v5215-objs := e1000_82575.o e1000_i210.o

# core driver files
igb_v5215-objs += igb_main.o e1000_mac.o e1000_nvm.o e1000_phy.o \
	    e1000_manage.o igb_param.o igb_ethtool.o kcompat.o e1000_api.o \
	    e1000_mbx.o igb_vmdq.o igb_procfs.o igb_hwmon.o igb_debugfs.o

# extra flags for module builds
#EXTRA_CFLAGS += -DDEBUG
EXTRA_CFLAGS += -DDRIVER_IGB
EXTRA_CFLAGS += -DDRIVER_NAME=igb
EXTRA_CFLAGS += -DDRIVER_NAME_CAPS=IGB

ifeq ($(CONFIG_PTP_1588_CLOCK),y)
igb_v5215-objs += igb_ptp.o
endif
