#!/usr/bin/env bash

module="axi_dma_dev"

rm /dev/${module}
/sbin/rmmod  $module
