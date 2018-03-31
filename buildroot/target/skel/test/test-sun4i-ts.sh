#!/bin/sh

run_ts_test()
{
	export TSLIB_TSEVENTTYPE=H3600
	export TSLIB_CONSOLEDEVICE=none
	export TSLIB_FBDEVICE=/dev/fb0
	export TSLIB_TSDEVICE=/dev/input/$1
	export TSLIB_CALIBFILE=/etc/pointercal
	export TSLIB_CONFFILE=/etc/ts.conf
	export TSLIB_PLUGINDIR=/usr/lib/ts
	ts_calibrate
	/test/td -t 30 -s 9 -c "/usr/bin/ts_test"
	echo "Finished"
}


for item in $(ls /sys/class/input |grep event)
	do
	printf "$item ->"
	
	DNAME=`cat /sys/class/input/$item/device/name`
	printf "$DNAME ->"

	case $DNAME in
	  sun4i-keyboard)
	    echo "skip"
	    ;;
	  sun4i-ir)
	    echo "skip"
	    ;;
	  sun4i-ts)
            echo "sun4i-ts: $item"
	    run_ts_test $item
	    ;;
	  *ts*)
            echo "skip"
	    ;;
	  *)
            echo "skip"
	    ;;
	esac
done




