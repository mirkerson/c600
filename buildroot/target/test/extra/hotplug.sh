#!/bin/sh

if [ -z "$1" -o -z "$2" ]; then
echo unknown event
exit 1
fi

case $1 in
mmcblk*)
        MMCINFO=`(cd /dev; ls mmcblk[0-9] 2>/dev/null)`
		logw $MMCINFO
		if [ -z "$MMCINFO" ]; then
        	infow2 1 1 MMC "Failed"
		else
        	infow2 1 2 MMC "OK"
		fi
        ;;
sd*)
        DSKINFO=`(cd /dev; ls sd[a-z] 2>/dev/null)`
		logw $DSKINFO
		if [ -z "$DSKINFO" ]; then
        	infow2 2 1 USB "Failed"
		else
        	infow2 2 2 USB "OK"
		fi
        ;;
esac

