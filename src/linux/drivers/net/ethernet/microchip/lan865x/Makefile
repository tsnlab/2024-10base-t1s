# SPDX-License-Identifier: GPL-2.0-only
#
# Makefile for the Microchip LAN865x Driver
#

KBUILD_CFLAGS := $(filter-out -mgeneral-regs-only, $(KBUILD_CFLAGS))

EXTRA_CFLAGS += -D__TSN_PTP__

obj-$(CONFIG_LAN865X) += lan865x_mod.o lan865x_arch.o lan865x_ptp.o

ifeq ($(LAN865X_DEBUG),1)
	EXTRA_CFLAGS += -D__LAN865X_DEBUG__
endif
