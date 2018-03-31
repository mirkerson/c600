#!/bin/ash

export TSLIB_TSEVENTTYPE=H3600
export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fb0
export TSLIB_TSDEVICE=/dev/input/event2
export TSLIB_CALIBFILE=/etc/pointercal
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_PLUGINDIR=/usr/lib/ts
ts_calibrate


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
	  /test/td -t 30 -s 9 -c "/usr/bin/evtest /dev/input/$item"
	  ;;
	  sun4i-ir)
	  echo "#######################TEST IR#########################"
	  echo "INFO: $item, $DNAME"
	  /test/td -t 30 -s 9 -c "/usr/bin/evtest /dev/input/$item"
	  ;;
	  *ts*)
	  echo "#######################TEST TS#########################"
	  echo "INFO: $item, $DNAME"
	  /test/td -t 30 -s 9 -c "/usr/bin/evtest /dev/input/$item"
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
/test/td -t 15 -s 9 -c "/test/gpu/lightshow/lightshow -w 800 -h 480"

printf "\n\n#########################################################\n\n"

printf "\n\n#### Test CedarX(LCD) ####\n\n"
/test/td -t 30 -s 6 -c "/test/CedarXPlayerTest-1.4.1 /root/test1.mp4"

printf "\n\n#########################################################\n\n"

printf "\n\n#### Test CedarX(HDMI) ####\n\n"
/test/display/fb_test.dat -o 4 9
printf "\n\n#### Test CedarX ####\n\n"
/test/td -t 30 -s 6 -c "/test/CedarXPlayerTest-1.4.1 /root/test1.mp4"
/test/display/fb_test.dat -o 1 0

printf "\n\n#########################################################\n\n"

echo "Test Network start"
sleep 3

ifconfig eth0 hw ether 3a:9d:33:46:d2:22
ifconfig eth0 192.168.3.123
ifconfig lo 127.0.0.1
ping -c 10 192.168.3.1

echo 7 > /proc/sys/kernel/printk






