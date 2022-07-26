#!/usr/bin/env bash

module="axi_dma_dev"

/sbin/insmod ./$module.ko
major=$(awk -v mod=$module '$2==mod{print $1}' /proc/devices)
mknod /dev/${module} c $major 0
chmod 777 /dev/${module}