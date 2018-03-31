#!/bin/sh

##################################################################
#
# Just for test. You can use this script to run android under linux
# . for example:
# #./run_android.sh -r root.img -s system.img
#
##################################################################


ROOTIMG=""
SYSIMG=""

show_help()
{
    printf "\n-r root.img\n"
    printf "-s system.img\n\n"

}

while getopts r:s:h OPTION
do
    case $OPTION in
	h) show_help
	    exit 1
	    ;;
	r) ROOTIMG=$OPTARG
	    ;;
	s) SYSIMG=$OPTARG
	    ;;
	*) show_help
	    exit 1
esac
done

if [ -z "$ROOTIMG" ]; then
	show_help
	exit 1
fi

if [ -z "$SYSIMG" ]; then
	show_help
	exit 1
fi


rm -rf /tmp/android
mkdir /tmp/android

mount -o loop $ROOTIMG /tmp/android
mount -o loop $SYSIMG /tmp/android/system

mkdir /tmp/android/data
mount -t tmpfs tmpfs /tmp/android/data


chroot /tmp/android ./init

