#!/bin/bash

# Mount the rootfs image, copy the kernel module, and unmount
mount_and_copy_module() {
  sudo mount -o loop rootfs.img fs
  sudo cp minibinder.ko fs
  sudo umount fs
}

# Start the QEMU VM with the specified kernel and rootfs
start_vm() {
  qemu-system-x86_64 -kernel bzImage -hda rootfs.img -append "root=/dev/sda console=ttyS0" -nographic
}

# Usage:
#   source ./dev_vm_utils.sh
#   mount_and_copy_module
#   start_vm