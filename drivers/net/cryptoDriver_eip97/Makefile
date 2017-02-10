
# Make sure that ARCH, KERNEL_DIR and CROSS_COMPILE are set

ifndef KERNEL_DIR
  ERROR_KERNEL_DIR_NOT_SET:=YES
endif

ifndef ARCH
  ERROR_ARCH_NOT_SET:=YES
endif

#ifeq ($(KERNELRELEASE),)
#   PWD:=$(shell pwd)
#endif
PWD :=drivers/net/cryptoDriver_eip97
SRC_APP=./source
SRC_UTILS=./source/utils

PATH_BUILD=$(PWD)

# Include Paths
INCLUDE_APP:= \
 -I$(PATH_BUILD)/include -I$(PATH_BUILD)/source/utils/global_ctrl/incl \
 -I$(PATH_BUILD)/source/utils/ring_ctrl/incl -I$(PATH_BUILD)/source/utils/ring_ctrl/EIP201/incl -I$(PATH_BUILD)/source/utils/sa_builder/incl \
 -I$(PATH_BUILD)/source/utils/token_builder/incl	\
 -I$(ROOTDIR)/$(LINUXDIR)/include/net

INCLUDE_FLAGS_ALL:= \
  $(INCLUDE_APP)


# Compiler Flags
WARNING_FLAGS=
CODE_COVERAGE_FLAGS=
PROFILING_FLAGS=
BACKWARD_COMPATIBILITY_FLAGS=

EXTRA_CFLAGS += $(BACKWARD_COMPATIBILITY_FLAGS)
EXTRA_CFLAGS += $(INCLUDE_FLAGS_ALL)
EXTRA_CFLAGS += $(WARNING_FLAGS)
EXTRA_CFLAGS += $(DEBUG_FLAGS)
EXTRA_CFLAGS += $(CODE_COVERAGE_FLAGS)
EXTRA_CFLAGS += $(PROFILING_FLAGS)
EXTRA_CFLAGS += -DMTK_CRYPTO_DRIVER -DMTK_EIP97_DRIVER -DDDK_PEC_IF_ID=4 -DLOG_SEVERITY_MAX=1
#-DMCRYPTO_DBG

EXTRA_CFLAGS +=-DEIP97_BUS_VERSION_AXI3

ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
EXTRA_CFLAGS   += -DWORKQUEUE_BH
endif

.PHONY = clean clean_module

obj-m	+= crypto_eip97_k.o
crypto_eip97_k-objs:= \
	$(SRC_UTILS)/mtk_eip97_device_lkm.o \
  $(SRC_UTILS)/mtk_eip97_dmabuf.o \
  $(SRC_UTILS)/mtk_eip97_dmares_lkm.o \
  $(SRC_UTILS)/mtk_eip97_ringhelper.o \
  $(SRC_APP)/mtk_driver97_init.o \
  $(SRC_APP)/mtk_eip97_interrupts.o \
  $(SRC_APP)/mtk_eip97_pecInit.o \
  $(SRC_APP)/mtk_eip97_pktProcess.o \
  $(SRC_APP)/mtk_global_init.o \
  $(SRC_APP)/mtk_global_eip97.o \
	$(SRC_APP)/mtk_init.o \
	$(SRC_UTILS)/token_builder/src/token_builder_context.o \
	$(SRC_UTILS)/token_builder/src/token_builder_core.o	\
	$(SRC_UTILS)/sa_builder/src/sa_builder.o	\
	$(SRC_UTILS)/sa_builder/src/sa_builder_ipsec.o	\
	$(SRC_UTILS)/sa_builder/src/sa_builder_basic.o	\
	$(SRC_UTILS)/ring_ctrl/src/eip202_cd_format.o	\
	$(SRC_UTILS)/ring_ctrl/src/eip202_cdr_dscr.o	\
	$(SRC_UTILS)/ring_ctrl/src/eip202_cdr_event.o	\
	$(SRC_UTILS)/ring_ctrl/src/eip202_cdr_fsm.o	\
	$(SRC_UTILS)/ring_ctrl/src/eip202_cdr_init.o	\
	$(SRC_UTILS)/ring_ctrl/src/eip202_rd_format.o	\
	$(SRC_UTILS)/ring_ctrl/src/eip202_rdr_dscr.o	\
	$(SRC_UTILS)/ring_ctrl/src/eip202_rdr_event.o \
	$(SRC_UTILS)/ring_ctrl/src/eip202_rdr_fsm.o	\
	$(SRC_UTILS)/ring_ctrl/src/eip202_rdr_init.o	\
	$(SRC_UTILS)/ring_ctrl/EIP201/src/eip201.o	\
	$(SRC_UTILS)/global_ctrl/src/eip97_global_fsm.o	\
	$(SRC_UTILS)/global_ctrl/src/eip97_global_init.o	\
	$(SRC_UTILS)/global_ctrl/src/eip97_global_prng.o \
	$(SRC_UTILS)/global_ctrl/src/eip97_global_event.o \
	$(SRC_UTILS)/log.o \
	$(SRC_UTILS)/dmares_gen.o \
	$(SRC_APP)/mtk_eip97_init.o \
	$(SRC_UTILS)/mtk_eip97_ring_eip202.o \
	$(SRC_UTILS)/mtk_eip97_token.o

clean:	clean_module

clean_module:
	# this is run in $PATH_BUILD where the Makefile resides
	find . \( -name "*.o" -o -name ".*.cmd" -o -name "*.bak"  \) -type f -print | xargs rm -f
	rm -rf .tmp_versions
	rm -f *.o  .*.cmd  .*.flags  *.mod.c  *.ko  *.symvers  *.markers  *.order
	
# end of file	
