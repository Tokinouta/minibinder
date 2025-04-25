#!/bin/bash

qemu-system-x86_64 -kernel bzImage -hda rootfs.img -append "root=/dev/sda console=ttyS0" -nographic
