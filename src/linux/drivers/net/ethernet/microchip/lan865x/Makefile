# SPDX-License-Identifier: GPL-2.0-only
#
# Makefile for the Microchip LAN865x Driver
#

obj-m += lan865x.o
KDIR ?= $(PWD)

all:
	make -C ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- $(KDIR) M=$(PWD) modules

clean:
	make -C ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- $(KDIR) M=$(PWD) clean
