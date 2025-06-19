# SPDX-License-Identifier: GPL-2.0-only
#
# Makefile for the Microchip LAN865x Driver
#

# obj-m += lan865x.o
obj-m += lan865x.o

lan865x-objs := lan865x_mod.o lan865x_arch.o

KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
