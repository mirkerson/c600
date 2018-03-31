# scripts/mkcmd.sh
#
# (c) Copyright 2013
# Allwinner Technology Co., Ltd. <www.allwinnertech.com>
# James Deng <csjamesdeng@allwinnertech.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# Notice:
#   1. This script muse source at the top directory of lichee.

BUILD_CONFIG=.buildconfig
cpu_cores=`cat /proc/cpuinfo | grep "processor" | wc -l`
if [ ${cpu_cores} -le 8 ] ; then
	LICHEE_JLEVEL=${cpu_cores}
else
	LICHEE_JLEVEL=`expr ${cpu_cores} / 2`
fi

export LICHEE_JLEVEL

function mk_error()
{
	echo -e "\033[47;31mERROR: $*\033[0m"
}

function mk_warn()
{
	echo -e "\033[47;34mWARN: $*\033[0m"
}

function mk_info()
{
	echo -e "\033[47;30mINFO: $*\033[0m"
}

# define importance variable
LICHEE_TOP_DIR=`pwd`
LICHEE_BR_DIR=${LICHEE_TOP_DIR}/buildroot
LICHEE_KERN_DIR=${LICHEE_TOP_DIR}/${LICHEE_KERN_VER}
LICHEE_TOOLS_DIR=${LICHEE_TOP_DIR}/tools
LICHEE_OUT_DIR=${LICHEE_TOP_DIR}/out
MKRULE_FILE=${LICHEE_TOOLS_DIR}/build/mkrule

# make surce at the top directory of lichee
if [ ! -d ${LICHEE_KERN_DIR} -o \
	! -d ${LICHEE_TOOLS_DIR} ] ; then
	mk_error "You are not at the top directory of lichee."
	mk_error "Please changes to that directory."
	exit 1
fi

# export importance variable
export LICHEE_TOP_DIR
export LICHEE_BR_DIR
export LICHEE_KERN_DIR
export LICHEE_TOOLS_DIR
export LICHEE_OUT_DIR

platforms=(
"android"
"dragonboard"
"linux"
"camdroid"
)

function check_env()
{
	if [ -z "${LICHEE_CHIP}" -o \
		-z "${LICHEE_PLATFORM}" -o \
		-z "${LICHEE_KERN_VER}" -o \
		-z "${LICHEE_BOARD}" ] ; then
		mk_error "run './build.sh config' setup env"
		exit 1
	fi
}

function init_defconf()
{
	local pattern
	local defconf
	local out_dir="common"

	check_env

	pattern="${LICHEE_CHIP}_${LICHEE_PLATFORM}_${LICHEE_BOARD}"
	defconf=`awk '$1=="'$pattern'" {print $2,$3}' ${MKRULE_FILE}`
	if [ -n "${defconf}" ] ; then
		export LICHEE_BR_DEFCONF=`echo ${defconf} | awk '{print $1}'`
		export LICHEE_KERN_DEFCONF=`echo ${defconf} | awk '{print $2}'`
		out_dir="${LICHEE_BOARD}"
	else
		pattern="${LICHEE_CHIP}_${LICHEE_PLATFORM}"
		defconf=`awk '$1=="'$pattern'" {print $2,$3}' ${MKRULE_FILE}`
		if [ -n "${defconf}" ] ; then
			export LICHEE_BR_DEFCONF=`echo ${defconf} | awk '{print $1}'`
			export LICHEE_KERN_DEFCONF=`echo ${defconf} | awk '{print $2}'`
			out_dir="common"
		fi
	fi

	export LICHEE_PLAT_OUT="${LICHEE_OUT_DIR}/${LICHEE_CHIP}/${LICHEE_PLATFORM}/${out_dir}"
	export LICHEE_BR_OUT="${LICHEE_PLAT_OUT}/buildroot"
	mkdir -p ${LICHEE_BR_OUT}
}

function init_chips()
{
	local chip=$1
	local cnt=0
	local ret=1

	for chipdir in ${LICHEE_TOOLS_DIR}/pack/chips/* ; do
		chips[$cnt]=`basename $chipdir`
		if [ "x${chips[$cnt]}" = "x${chip}" ] ; then
			ret=0
			export LICHEE_CHIP=${chip}
		fi
		((cnt+=1))
	done

	return ${ret}
}

function init_platforms()
{
	local cnt=0
	local ret=1
	local platform=""

	for platform in ${platforms[@]} ; do
		if [ "x${platform}" = "x$1" ] ; then
			ret=0
			export LICHEE_PLATFORM=${platform}
		fi
		((cnt+=1))
	done

	return ${ret}
}

function init_kern_ver()
{
	local kern_ver=$1
	local cnt=0
	local ret=1

	for kern_dir in ${LICHEE_TOP_DIR}/linux-* ; do
		kern_vers[$cnt]=`basename $kern_dir`
		if [ "x${kern_vers[$cnt]}" = "x${kern_ver}" ] ; then
			ret=0
			export LICHEE_KERN_VER=${kern_ver}
		fi
		((cnt+=1))
	done

	return ${ret}
}

function init_boards()
{
	local chip=$1
	local platform=$2
	local kern_ver=$3
	local board=$4
	local cnt=0
	local ret=1

	for boarddir in ${LICHEE_TOOLS_DIR}/pack/chips/${chip}/configs/${platform}/* ; do
		boards[$cnt]=`basename $boarddir`
		if [ "x${boards[$cnt]}" = "x${board}" ] ; then
			ret=0
			export LICHEE_BOARD=${board}
		fi
		((cnt+=1))
	done

	return ${ret}
}

function select_chip()
{
	local cnt=0
	local choice
	local call=$1

	printf "All available chips:\n"
	for chipdir in ${LICHEE_TOOLS_DIR}/pack/chips/* ; do
		chips[$cnt]=`basename $chipdir`
		printf "%4d. %s\n" $cnt ${chips[$cnt]}
		((cnt+=1))
	done

	while true ; do
		read -p "Choice: " choice
		if [ -z "${choice}" ] ; then
			continue
		fi

		if [ -z "${choice//[0-9]/}" ] ; then
			if [ $choice -ge 0 -a $choice -lt $cnt ] ; then
				export LICHEE_CHIP="${chips[$choice]}"
				echo "export LICHEE_CHIP=${chips[$choice]}" >> ${BUILD_CONFIG}
				break
			fi
		fi
		printf "Invalid input ...\n"
	done
}

function select_platform()
{
	local cnt=0
	local choice
	local call=$1
	local platform=""

	select_chip

	printf "All available platforms:\n"
	for platform in ${platforms[@]} ; do
		printf "%4d. %s\n" $cnt $platform
		((cnt+=1))
	done

	while true ; do
		read -p "Choice: " choice
		if [ -z "${choice}" ] ; then
			continue
		fi

		if [ -z "${choice//[0-9]/}" ] ; then
			if [ $choice -ge 0 -a $choice -lt $cnt ] ; then
				export LICHEE_PLATFORM="${platforms[$choice]}"
				echo "export LICHEE_PLATFORM=${platforms[$choice]}" >> ${BUILD_CONFIG}
				break
			fi
		fi
		printf "Invalid input ...\n"
	done
}

function select_kern_ver()
{
	local cnt=0
	local choice

	select_platform

	printf "All available kernel:\n"
	for kern_dir in ${LICHEE_TOP_DIR}/linux-* ; do
		kern_vers[$cnt]=`basename $kern_dir`
		printf "%4d. %s\n" $cnt ${kern_vers[$cnt]}
		((cnt+=1))
	done

	while true ; do
		read -p "Choice: " choice
		if [ -z "${choice}" ] ; then
			continue
		fi

		if [ -z "${choice//[0-9]/}" ] ; then
			if [ $choice -ge 0 -a $choice -lt $cnt ] ; then
				export LICHEE_KERN_VER="${kern_vers[$choice]}"
				echo "export LICHEE_KERN_VER=${kern_vers[$choice]}" >> ${BUILD_CONFIG}
				break
			fi
		fi
		printf "Invalid input ...\n"
	done
}

function select_board()
{
	local cnt=0
	local choice

	select_kern_ver

	printf "All available boards:\n"
	for boarddir in ${LICHEE_TOOLS_DIR}/pack/chips/${LICHEE_CHIP}/configs/* ; do
		boards[$cnt]=`basename $boarddir`
		if [ "x${boards[$cnt]}" = "xdefault" ] ; then
			continue
		fi
		printf "%4d. %s\n" $cnt ${boards[$cnt]}
		((cnt+=1))
	done

	while true ; do
		read -p "Choice: " choice
		if [ -z "${choice}" ] ; then
			continue
		fi

		if [ -z "${choice//[0-9]/}" ] ; then
			if [ $choice -ge 0 -a $choice -lt $cnt ] ; then
				export LICHEE_BOARD="${boards[$choice]}"
				echo "export LICHEE_BOARD=${boards[$choice]}" >> ${BUILD_CONFIG}
				break
			fi
		fi
		printf "Invalid input ...\n"
	done
}

function mkbr()
{
	mk_info "build buildroot ..."

	local build_script="scripts/build.sh"

	prepare_toolchain

	(cd ${LICHEE_BR_DIR} && [ -x ${build_script} ] && ./${build_script})
	[ $? -ne 0 ] && mk_error "build buildroot Failed" && return 1

	mk_info "build buildroot OK."
}

function clbr()
{
	mk_info "build buildroot ..."

	local build_script="scripts/build.sh"
	(cd ${LICHEE_BR_DIR} && [ -x ${build_script} ] && ./${build_script} "clean")

	mk_info "clean buildroot OK."
}

function prepare_toolchain()
{
	local ARCH="arm";
	local GCC="";
	local GCC_PREFIX="";
	local toolchain_archive="${LICHEE_BR_DIR}/dl/gcc-linaro-arm-linux-gnueabihf-4.9-2014.08_linux.tar.xz";
	local tooldir="";

	mk_info "Prepare toolchain ..."

	if [ -n "`echo ${LICHEE_CHIP} | grep "sun5[0-9]i"`" ]; then
		ARCH="aarch64"
		toolchain_archive="${LICHEE_TOOLS_DIR}/build/toolchain/gcc-linaro-aarch64.tar.xz";
	fi

	tooldir=${LICHEE_OUT_DIR}/external-toolchain/gcc-${ARCH}
	if [ ! -d "${tooldir}" ]; then
		mkdir -p ${tooldir} || exit 1
		tar --strip-components=1 -xf ${toolchain_archive} -C ${tooldir} || exit 1
	fi


	GCC=$(find ${tooldir} -perm /a+x -a -regex '.*-gcc');
	if [ -z "${GCC}" ]; then
		tar --strip-components=1 -xf ${toolchain_archive} -C ${tooldir} || exit 1
		GCC=$(find ${tooldir} -perm /a+x -a -regex '.*-gcc');
	fi
	GCC_PREFIX=${GCC##*/};

	if [ "${tooldir}" == "${LICHEE_TOOLCHAIN_PATH}" \
		-a "${LICHEE_CROSS_COMPILER}-gcc" == "${GCC_PREFIX}" \
		-a -x "${GCC}" ]; then
		return
	fi

	if ! echo $PATH | grep -q "${tooldir}" ; then
		export PATH=${tooldir}/bin:$PATH
	fi


	LICHEE_CROSS_COMPILER="${GCC_PREFIX%-*}";

	if [ -n ${LICHEE_CROSS_COMPILER} ]; then
		if [ -f ${BUILD_CONFIG} ]; then
			sed -i '/LICHEE_CROSS_COMPILER.*/d' ${BUILD_CONFIG}
			sed -i '/LICHEE_TOOLCHAIN_PATH.*/d' ${BUILD_CONFIG}
		fi
		export LICHEE_CROSS_COMPILER=${LICHEE_CROSS_COMPILER}
		export LICHEE_TOOLCHAIN_PATH=${tooldir}
		echo "export LICHEE_CROSS_COMPILER=${LICHEE_CROSS_COMPILER}" >> ${BUILD_CONFIG}
		echo "export LICHEE_TOOLCHAIN_PATH=${tooldir}" >> ${BUILD_CONFIG}
	fi
}

function mkkernel()
{
	mk_info "build kernel ..."

	local build_script="scripts/build.sh"

	prepare_toolchain

	# mark kernel .config belong to which platform
	local config_mark="${LICHEE_KERN_DIR}/.config.mark"
	if [ -f ${config_mark} ] ; then
		if ! grep -q "${LICHEE_PLATFORM}" ${config_mark} ; then
			mk_info "clean last time build for different platform"
			(cd ${LICHEE_KERN_DIR} && [ -x ${build_script} ] && ./${build_script} "clean")
			echo "${LICHEE_PLATFORM}" > ${config_mark}
		fi
	else
		echo "${LICHEE_PLATFORM}" > ${config_mark}
	fi

	(cd ${LICHEE_KERN_DIR} && [ -x ${build_script} ] && ./${build_script})
	[ $? -ne 0 ] && mk_error "build kernel Failed" && return 1

	mk_info "build kernel OK."
}

function clkernel()
{
	mk_info "clean kernel ..."

	local build_script="scripts/build.sh"

	prepare_toolchain

	(cd ${LICHEE_KERN_DIR} && [ -x ${build_script} ] && ./${build_script} "clean")

	mk_info "clean kernel OK."
}

function mkboot()
{
	mk_info "build boot ..."
	mk_info "build boot OK."
}

function packtinyandroid()
{
	rm -rf rootfs_tinyandroid
	mkdir -p rootfs_tinyandroid
	cp -a ${LICHEE_BR_OUT}/target/* rootfs_tinyandroid/
	if [ -f $LICHEE_KERN_DIR/tinyandroid.tar.gz ];then
		mk_info "copy tinyandroid"
		cd $LICHEE_KERN_DIR && tar zvxf tinyandroid.tar.gz && cd -
		rm -rf rootfs_tinyandroid/init
		rm -rf rootfs_tinyandroid/linuxrc
		cp -a  $LICHEE_KERN_DIR/tinyandroid/*  rootfs_tinyandroid/
		if [ "x$PACK_BSPTEST" = "xtrue" ];then
			if [ -d ${LICHEE_TOP_DIR}/SATA/linux/target ];then
				mk_info "copy SATA tinyandroid"
				cp -a ${LICHEE_TOP_DIR}/SATA/linux/target  rootfs_tinyandroid/
			fi
		fi
	fi
	mk_info "generating rootfs..."

	NR_SIZE=`du -sm rootfs_tinyandroid | awk '{print $1}'`
	NEW_NR_SIZE=$(((($NR_SIZE+32)/16)*16))
	#NEW_NR_SIZE=360
	TARGET_IMAGE=rootfs.ext4

	echo "blocks: $NR_SIZE"M" -> $NEW_NR_SIZE"M""
	$LICHEE_BR_DIR/target/tools/host/usr/bin/make_ext4fs -l \
		$NEW_NR_SIZE"M" $TARGET_IMAGE rootfs_tinyandroid/
	fsck.ext4 -y $TARGET_IMAGE > /dev/null
	echo "success in generating rootfs"

	if [ -d $LICHEE_PLAT_OUT ]; then
		cp -v $TARGET_IMAGE $LICHEE_PLAT_OUT/
	fi

	echo "Build at: `date`"
}

function mk_defroot()
{
	local ROOTFS=${LICHEE_PLAT_OUT}/rootfs_def
	local INODES=""
	local BLOCKS=""

	mk_info "Build default rootfs ..."
	if [ "$1" = "f" ]; then
		rm -fr ${ROOTFS}
	fi

	if [ ! -f ${ROOTFS} ]; then
		mkdir -p ${ROOTFS}
		tar -jxf ${LICHEE_TOOLS_DIR}/build/rootfs_tar/target.tar.bz2 -C ${ROOTFS}
	fi

	mkdir -p ${ROOTFS}/lib/modules
	cp -rf ${LICHEE_KERN_DIR}/output/lib/modules/* \
		${ROOTFS}/lib/modules/

	(cd ${ROOTFS}; ln -fs bin/busybox init)

	# calculate needed inodes
	INODES=$(find $ROOTFS | wc -l)
	INODES=$(expr $INODES + 400)

	# calculate needed blocks
	# size ~= superblock, block+inode bitmaps, inodes (8 per block), blocks
	# we scale inodes / blocks with 10% to compensate for bitmaps size + slack
	BLOCKS=$(du -s -c -k $ROOTFS | grep total | sed -e "s/total//")
	BLOCKS=$(expr 500 + \( $BLOCKS + $INODES / 8 \) \* 11 / 10)
	# we add 1300 blocks (a bit more than 1 MiB, assuming 1KiB blocks) for
	# the journal if ext3/4
	# Note: I came to 1300 blocks after trial-and-error checks. YMMV.
	BLOCKS=$(expr 1300 + $BLOCKS )

cat  > ${LICHEE_PLAT_OUT}/.rootfs << EOF
chown -h -R 0:0 ${ROOTFS}
${LICHEE_TOOLS_DIR}/build/bin/makedevs -d \
	${LICHEE_TOOLS_DIR}/build/rootfs_tar/_device_table.txt ${ROOTFS}
${LICHEE_TOOLS_DIR}/build/bin/genext2fs -q -d \
	${ROOTFS} ${LICHEE_PLAT_OUT}/rootfs.ext4 -N ${INODES} -b ${BLOCKS}
EOF

	chmod a+x ${LICHEE_PLAT_OUT}/.rootfs
	fakeroot -- ${LICHEE_PLAT_OUT}/.rootfs
	fsck.ext4 -y ${LICHEE_PLAT_OUT}/rootfs.ext4
}

function mkrootfs()
{
	mk_info "build rootfs ..."

	if [ ${LICHEE_PLATFORM} = "linux" ] ; then

		if [ ${SKIP_BR} -ne 0 ]; then
			mk_defroot $1
		else
			make O=${LICHEE_BR_OUT} -C ${LICHEE_BR_DIR} \
				BR2_TOOLCHAIN_EXTERNAL_PATH=${LICHEE_TOOLCHAIN_PATH} \
				BR2_TOOLCHAIN_EXTERNAL_PREFIX=${LICHEE_CROSS_COMPILER} \
				BR2_JLEVEL=${LICHEE_JLEVEL} target-post-image

			[ $? -ne 0 ] && mk_error "build rootfs Failed" && return 1

			cp ${LICHEE_BR_OUT}/images/rootfs.ext4 ${LICHEE_PLAT_OUT}

			if [ -f "${LICHEE_BR_OUT}/images/rootfs.squashfs" ]; then
				cp ${LICHEE_BR_OUT}/images/rootfs.squashfs ${LICHEE_PLAT_OUT}
			fi
		fi

		if [ "x$PACK_TINY_ANDROID" = "xtrue" ];then
			packtinyandroid
		fi
	elif [ ${LICHEE_PLATFORM} = "dragonboard" ] ; then
		echo "Regenerating dragonboard Rootfs..."
		(
		cd ${LICHEE_BR_DIR}/target/dragonboard; \
			if [ ! -d "./rootfs" ]; then \
				echo "extract dragonboard rootfs.tar.gz"; \
				tar zxf rootfs.tar.gz; \
			fi
		)
		mkdir -p ${LICHEE_BR_DIR}/target/dragonboard/rootfs/lib/modules
		rm -rf ${LICHEE_BR_DIR}/target/dragonboard/rootfs/lib/modules/*
		cp -rf ${LICHEE_KERN_DIR}/output/lib/modules/* \
			${LICHEE_BR_DIR}/target/dragonboard/rootfs/lib/modules/
		(cd ${LICHEE_BR_DIR}/target/dragonboard; ./build.sh)
		cp ${LICHEE_BR_DIR}/target/dragonboard/rootfs.ext4 ${LICHEE_PLAT_OUT}
	else
		mk_info "skip make rootfs for ${LICHEE_PLATFORM}"
	fi

	mk_info "build rootfs OK."
}

function mklichee()
{

	mk_info "----------------------------------------"
	mk_info "build lichee ..."
	mk_info "chip: $LICHEE_CHIP"
	mk_info "platform: $LICHEE_PLATFORM"
	mk_info "kernel: $LICHEE_KERN_VER"
	mk_info "board: $LICHEE_BOARD"
	mk_info "output: out/${LICHEE_CHIP}/${LICHEE_PLATFORM}/${LICHEE_BOARD}"
	mk_info "----------------------------------------"

	check_env

	if [ ${SKIP_BR} -eq 0 ]; then
		mkbr
	fi

	mkkernel && mkrootfs $1

	[ $? -ne 0 ] && return 1

	mk_info "----------------------------------------"
	mk_info "build lichee OK."
	mk_info "----------------------------------------"
}

function mkclean()
{
	clkernel

	mk_info "clean product output in ${LICHEE_PLAT_OUT} ..."
	cd ${LICHEE_PLAT_OUT}
	ls | grep -v "buildroot" | xargs rm -rf
	cd - > /dev/null

}

function mkdistclean()
{
	clkernel
	if [ ${SKIP_BR} -eq 0 ]; then
		clbr
	fi

	mk_info "clean entires output dir ..."
	rm -rf ${LICHEE_OUT_DIR}
}

function mkpack()
{
	mk_info "packing firmware ..."

	check_env

	(cd ${LICHEE_TOOLS_DIR}/pack && \
		./pack -c ${LICHEE_CHIP} -p ${LICHEE_PLATFORM} -b ${LICHEE_BOARD} -k ${LICHEE_KERN_VER} $@)
}

function mkhelp()
{
	printf "
	mkscript - lichee build script

	<version>: 1.0.0
	<author >: james

	<command>:
	mkboot      build boot
	mkbr        build buildroot
	mkkernel    build kernel
	mkrootfs    build rootfs for linux, dragonboard
	mklichee    build total lichee

	mkclean     clean current board output
	mkdistclean clean entires output

	mkpack      pack firmware for lichee

	mkhelp      show this message

	"
}

