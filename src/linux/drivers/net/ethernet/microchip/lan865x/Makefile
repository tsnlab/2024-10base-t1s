# SPDX-License-Identifier: GPL-2.0-only
#
# Makefile for the Microchip LAN865x Driver
#

obj-$(CONFIG_LAN865X) += lan865x.o lan865x_arch.o lan865x_ptp.o

ifeq ($(LAN865X_DEBUG),1)
	EXTRA_CFLAGS += -D__LAN865X_DEBUG__
endif
