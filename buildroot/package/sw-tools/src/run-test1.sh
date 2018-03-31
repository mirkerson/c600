#!/bin/ash


run_sun4i_rts()
{
	_CTP_USED=`sw_ctl -f /dev/sw_ctl -m ctp_para -s ctp_used`

	if [ "$CTP_USED" ! = "1" ]; then
        echo "###############################TEST RTS###############################"
        export TSLIB_TSEVENTTYPE=H3600
        export TSLIB_CONSOLEDEVICE=none
        export TSLIB_FBDEVICE=/dev/fb0
        export TSLIB_TSDEVICE=/dev/input/event2
        export TSLIB_CALIBFILE=/etc/pointercal
        export TSLIB_CONFFILE=/etc/ts.conf
        export TSLIB_PLUGINDIR=/usr/lib/ts
        ts_calibrate
	fi
}

CTP_USED=`sw_ctl -f /dev/sw_ctl -m ctp_para -s ctp_used`
CTP_NAME=`sw_ctl -f /dev/sw_ctl -m ctp_para -s ctp_name`

if [ "$CTP_USED" = "1" ]; then

case $CTP_NAME in
	ft5x_ts)
	echo "Load ft5x touch screen driver"
	(cd /lib/modules/`uname -r`/; insmod ft5x_ts.ko)
	;;
	Goodix-TS)
	echo "Load goodix touch screen driver"
        (cd /lib/modules/`uname -r`/; insmod goodix_touch.ko)
	;;
	*)
	echo "Unknown touch screen: $CTP_NAME"
	;;
esac

fi

echo "After 3 seconds, input test start"
sleep 3
echo "input test start"

for item in $(ls /sys/class/input |grep event)
	do
	printf "$item ->"
	
	DNAME=`cat /sys/class/input/$item/device/name`
	printf "$DNAME ->"

	case $DNAME in
	  sun4i-keyboard)
	  echo "#######################TEST KEY#########################"
	  echo "INFO: $item, $DNAME"
	  td -t 30 -s 9 -c "/usr/bin/evtest /dev/input/$item"
	  ;;
	  sun4i-ir)
	  echo "#######################TEST IR#########################"
	  echo "INFO: $item, $DNAME"
	  td -t 30 -s 9 -c "/usr/bin/evtest /dev/input/$item"
	  ;;
	  sun4i-ts)
	  test-sun4i-rts
	  ;;
	  ft5x_ts)
	  echo "#######################TEST TS#########################"
	  echo "INFO: $item, $DNAME"
	  td -t 30 -s 9 -c "/usr/bin/evtest /dev/input/$item"
	  ;;
	  *)
	  echo "#######################Skip############################"
	  echo "INFO: $item, $DNAME"
	  ;;
	esac
done


printf "\n\n#########################################################\n\n"

printf "\n\n#### Test GPU 3D ####\n\n"
insmod /lib/modules/`uname -r`/ump.ko
insmod /lib/modules/`uname -r`/mali.ko
td -t 15 -s 9 -c "/test/gpu/lightshow/lightshow -w 800 -h 480"

printf "\n\n#########################################################\n\n"

printf "\n\n#### Test CedarX(LCD) ####\n\n"
td -t 30 -s 6 -c "/bin/CedarXPlayerTest /root/test1.mp4"

printf "\n\n#########################################################\n\n"

printf "\n\n#### Test CedarX(HDMI) ####\n\n"
/test/display/fb_test.dat -o 4 9
printf "\n\n#### Test CedarX ####\n\n"
td -t 30 -s 6 -c "/bin/CedarXPlayerTest /root/test1.mp4"
/test/display/fb_test.dat -o 1 0

printf "\n\n#########################################################\n\n"

echo "Test Network start"
sleep 3

ifconfig eth0 hw ether 3a:9d:33:46:d2:22
ifconfig eth0 192.168.3.123
ifconfig lo 127.0.0.1
ping -c 10 192.168.3.1

