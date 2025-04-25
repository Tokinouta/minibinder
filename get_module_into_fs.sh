#!/bin/bash

sudo mount -o loop rootfs.img fs
sudo cp minibinder.ko fs
sudo umount fs