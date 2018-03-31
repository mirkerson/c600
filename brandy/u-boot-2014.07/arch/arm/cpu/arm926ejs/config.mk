#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#
#CROSS_COMPILE := $(src)/../gcc-linaro/bin/arm-linux-gnueabi-
CROSS_COMPILE:= arm-none-linux-gnueabi-
PLATFORM_CPPFLAGS += -march=armv5te
