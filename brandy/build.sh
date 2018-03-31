#!/bin/bash
set -e

PLATFORM="sun8iw3p1"
MODE=""

show_help()
{
	printf "\nbuild.sh - Top level build scritps\n"
	echo "Valid Options:"
	echo "  -h  Show help message"
	echo "  -p <platform> platform, e.g. sun5i, sun6i, sun8iw1p1, sun8iw3p1, sun9iw1p1"
	echo "  -m <mode> mode, e.g. ota_test"
	echo "  -t install gcc tools chain"
	printf "\n\n"
}

prepare_toolchain()
{
        local ARCH="arm";
        local GCC="";
        local GCC_PREFIX="";
        local toolchain_archive_aarch64="./toolchain/gcc-linaro-aarch64.tar.xz";
        local toolchain_archive_arm="./toolchain/gcc-linaro-arm.tar.xz";
        local tooldir_aarch64="./toolchain/gcc-aarch64";
        local tooldir_arm="./toolchain/gcc-arm";

        echo "Prepare toolchain ..."

        if [ ! -d "${tooldir_aarch64}" ]; then
                mkdir -p ${tooldir_aarch64} || exit 1
                tar --strip-components=1 -xf ${toolchain_archive_aarch64} -C ${tooldir_aarch64} || exit 1
        fi

        if [ ! -d "${tooldir_arm}" ]; then
                mkdir -p ${tooldir_arm} || exit 1
                tar --strip-components=1 -xf ${toolchain_archive_arm} -C ${tooldir_arm} || exit 1
        fi
}

build_uboot()
{
	if [ "x${PLATFORM}" = "xsun50iw1p1" ]; then

		prepare_toolchain
		#make atf
		cd arm-trusted-firmware-1.0/
		make clean && make PLAT=sun50iw1p1
		cd ..
        fi
	if [ "x${PLATFORM}" = "xsun50iw1p1" ] || [ "x${PLATFORM}" = "xsun8iw10p1" ] || [ "x${PLATFORM}" = "xsun8iw11p1" ]; then
                cd u-boot-2014.07/
	else
		cd u-boot-2011.09/
	fi

	make distclean
	if [ "x$MODE" = "xota_test" ] ; then
		export "SUNXI_MODE=ota_test"
	fi
	make ${PLATFORM}_config
	make -j16

    if  [ ${PLATFORM} = "sun8iw6p1" ] || \
	[ ${PLATFORM} = "sun8iw7p1" ] || \
	[ ${PLATFORM} = "sun8iw8p1" ] || \
	[ ${PLATFORM} = "sun9iw1p1" ] || \
	[ ${PLATFORM} = "sun8iw5p1" ] || \
	[ ${PLATFORM} = "sun50iw1p1" ] || \
	[ ${PLATFORM} = "sun8iw10p1" ] || \
	[ ${PLATFORM} = "sun8iw11p1" ]; then
        make spl
    fi

	cd - 1>/dev/null
}

while getopts p:m:t OPTION
do
	case $OPTION in
	p)
		PLATFORM=$OPTARG
		;;
	m)
		MODE=$OPTARG
		;;
	t)
		prepare_toolchain
		exit
		;;
	*) show_help
		exit
		;;
esac
done


build_uboot




