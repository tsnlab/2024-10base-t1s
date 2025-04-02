# The Linux kernel


## Select kernel build method

Please refer to the following url for how to build the kernel.


### Native builds

* [Download kernel source](https://www.raspberrypi.com/documentation/computers/linux_kernel.html#download-kernel-source)

* [install the build dependencies](https://www.raspberrypi.com/documentation/computers/linux_kernel.html#natively-build-a-kernel)

* [prepare the default configuration](https://www.raspberrypi.com/documentation/computers/linux_kernel.html#native-build-configuration)

    cd linux

    KERNEL=kernel8

    make bcm2711\_defconfig

    copy or overwrite the contents in the current repository to the downloaded Raspberry Pi Linux source


* [Customise the kernel version using LOCALVERSION](https://www.raspberrypi.com/documentation/computers/linux_kernel.html#native-customisation)

* [Build](https://www.raspberrypi.com/documentation/computers/linux_kernel.html#native-build)

* [Install the kernel](https://www.raspberrypi.com/documentation/computers/linux_kernel.html#native-install)

* Module installation

  Check if lan865x.ko is created in the drivers/net/ethernet/microchip/lan865x/ directory

  sudo cp ./drivers/net/ethernet/microchip/lan865x/lan865x.ko /lib/modules/$(uname -r)/kernel/drivers/net/ethernet/microchip/lan865x/

  sudo depmod -a

  sudo insmod ./drivers/net/ethernet/microchip/lan865x/lan865x.ko
  

### Cross-compilation

* [Download kernel source](https://www.raspberrypi.com/documentation/computers/linux_kernel.html#download-kernel-source)

* [Install required dependencies and toolchain](https://www.raspberrypi.com/documentation/computers/linux_kernel.html#cross-compiled-dependencies)

* [prepare the default configuration](https://www.raspberrypi.com/documentation/computers/linux_kernel.html#cross-compiled-build-configurationn)

    cd linux

    KERNEL=kernel8

    make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- bcm2711_defconfig

    copy or overwrite the contents in the current repository to the downloaded Raspberry Pi Linux source


* [Customise the kernel version using LOCALVERSION](https://www.raspberrypi.com/documentation/computers/linux_kernel.html#cross-compiled-customisation)

* [Build](https://www.raspberrypi.com/documentation/computers/linux_kernel.html#cross-compiled-build)

* [Install the kernel](https://www.raspberrypi.com/documentation/computers/linux_kernel.html#cross-compiled-install)

* Module installation

  Check if lan865x.ko is created in the drivers/net/ethernet/microchip/lan865x/ directory

  Copy the lan865x.ko file in drivers/net/ethernet/microchip/lan865x/ to an appropriate directory on the Raspberry Pi 4 board using USB, etc.

  sudo cp ./lan865x.ko /lib/modules/$(uname -r)/kernel/drivers/net/ethernet/microchip/lan865x/

  sudo depmod -a

  sudo insmod ./lan865x.ko

