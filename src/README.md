## **Download kernel source**
Before you can build for any target, you need the kernel source. To get the kernel source, you need Git. Begin by installing Git on your device, if you don’t already have it:

sudo apt install git
Next, download the source code for the latest Raspberry Pi kernel:

git clone --depth=1 https://github.com/raspberrypi/linux
This can take several minutes.

Now that you have the kernel source, build a fresh kernel natively or via cross-compilation.
(https://www.raspberrypi.com/documentation/computers/linux_kernel.html#kernel)

### **Install required dependencies and toolchain**
To build the sources for cross-compilation, install the required dependencies onto your device. Run the following command to install most dependencies:

sudo apt install bc bison flex libssl-dev make libc6-dev libncurses5-dev
Then, install the proper toolchain for the kernel architecture you wish to build:

To install the 64-bit toolchain to build a 64-bit kernel, run the following command:

sudo apt install crossbuild-essential-arm64
To install the 32-bit toolchain to build a 32-bit kernel, run the following command:

sudo apt install crossbuild-essential-armhf

### **Build configuration**
This section describes how to apply the default configuration when you build a kernel. You can also configure your kernel in the following ways:

enable and disable kernel features

apply patches from another source

To prepare the default configuration, run the appropriate commands from the table below for your Raspberry Pi model.

cd linux
KERNEL=kernel8
make bcm2711_defconfig

### **Customise the kernel version using this repository**
Overwrite or copy all files in this repository to your downloaded kernel source.

### **Build**
Run the following command to build a 64-bit kernel:

make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image modules dtbs
Run the following command to build a 32-bit kernel:

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- zImage modules dtbs

### **Install the kernel**
Having built the kernel, you need to copy it onto your Raspberry Pi boot media (likely an SD card or SSD) and install the modules.

**Find your boot media**
First, run lsblk. Then, connect your boot media. Run lsblk again; the new device represents your boot media. You should see output similar to the following:

sdb
   sdb1
   sdb2
If sdb represents your boot media, sdb1 represents the the FAT32-formatted boot partition and sdb2 represents the (likely ext4-formatted) root partition.

First, mount these partitions as mnt/boot and mnt/root, adjusting the partition letter to match the location of your boot media:

mkdir mnt
mkdir mnt/boot
mkdir mnt/root
sudo mount /dev/sdb1 mnt/boot
sudo mount /dev/sdb2 mnt/root
**Install**
Next, install the kernel modules onto the boot media:

For 64-bit kernels:

sudo env PATH=$PATH make -j12 ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- INSTALL_MOD_PATH=mnt/root modules_install
For 32-bit kernels:

sudo env PATH=$PATH make -j12 ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_MOD_PATH=mnt/root modules_install

Next, install the kernel and Device Tree blobs into the boot partition, backing up your original kernel.

To install the 64-bit kernel:

Run the following commands to create a backup image of the current kernel, install the fresh kernel image, overlays, README, and unmount the partitions:

sudo cp mnt/boot/$KERNEL.img mnt/boot/$KERNEL-backup.img
sudo cp arch/arm64/boot/Image mnt/boot/$KERNEL.img
sudo cp arch/arm64/boot/dts/broadcom/*.dtb mnt/boot/
sudo cp arch/arm64/boot/dts/overlays/*.dtb* mnt/boot/overlays/
sudo cp arch/arm64/boot/dts/overlays/README mnt/boot/overlays/
sudo umount mnt/boot
sudo umount mnt/root
To install the 32-bit kernel:

Run the following commands to create a backup image of the current kernel and install the fresh kernel image:

sudo cp mnt/boot/$KERNEL.img mnt/boot/$KERNEL-backup.img
sudo cp arch/arm/boot/zImage mnt/boot/$KERNEL.img
Depending on your kernel version, run the following command to install Device Tree blobs:

For kernels up to version 6.4:

sudo cp arch/arm/boot/dts/*.dtb mnt/boot/
For kernels version 6.5 and above:

sudo cp arch/arm/boot/dts/broadcom/*.dtb mnt/boot/
Finally, install the overlays and README, and unmount the partitions:

sudo cp arch/arm/boot/dts/overlays/*.dtb* mnt/boot/overlays/
sudo cp arch/arm/boot/dts/overlays/README mnt/boot/overlays/
sudo umount mnt/boot
sudo umount mnt/root
Finally, connect the boot media to your Raspberry Pi and connect it to power to run your freshly-compiled kernel.




# Linux Kernel

## Introduction

The Raspberry Pi kernel is hosted on GitHub; updates lag behind the upstream Linux kernel. The upstream kernel updates continuously, whereas Raspberry Pi integrates **long-term releases** of the Linux kernel into the Raspberry Pi kernel. We generate a `next` branch in raspberrypi/firmware for each long-term Linux kernel release. After extensive testing and discussion, we merge each `next` branch into the main branch of our repository.

## Update

The usual Raspberry Pi OS update process automatically updates your kernel to the latest stable release. If you want to try the latest unstable test kernel, you can manually update.

## Build the kernel

The default compilers and linkers distributed with an OS are configured to build executables to run on that OS.

**Native builds** use these default compilers and linkers. **Cross-compilation** is the process of building code for a target other than the one running the build process.

Cross-compilation of the Raspberry Pi kernel allows you to build a 64-bit kernel from a 32-bit OS, and vice versa. Alternatively, you can cross-compile a 32-bit or 64-bit Raspberry Pi kernel from a device other than a Raspberry Pi.

The instructions below are divided into native builds and cross-compilation. Choose the section appropriate for your situation; although the two processes share many steps, there are also some important differences.

### Download kernel source

Before you can build for any target, you need the kernel source. To get the kernel source, you need Git. Begin by installing Git on your device, if you don’t already have it:

$ sudo apt install git


Next, download the source code for the latest Raspberry Pi kernel:

$ git clone --depth=1 https://github.com/raspberrypi/linux


This can take several minutes.

| Tip | The To download a different branch with no history, add the For a full list of available branches, see the the Raspberry Pi kernel repository. |
| --- | --- |

Now that you have the kernel source, build a fresh kernel natively or via cross-compilation.

### Natively build a kernel

This guide assumes that your Raspberry Pi runs the latest version of Raspberry Pi OS.

First, install the build dependencies:


