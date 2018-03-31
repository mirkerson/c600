#!/bin/sh


#ITEM_LIST=""

while true; do

#is usb wifi
if [ "$1" -eq 0 ]; then

echo 1 > /proc/driver/mmc-pm/power
echo 1 > /proc/driver/sunxi-mmc.3/insert
sleep 2

if cat /sys/bus/mmc/devices/mmc*/type 2>/dev/null |grep SDIO 2>&1 1>/dev/null
then
	infow2 11 2 WIFI "OK(SDIO)"
else
	infow2 11 1 WIFI "Fail(SDIO)"
fi
echo 0 > /proc/driver/sunxi-mmc.3/insert
sleep 10

else
if ifconfig -a |grep wlan0
then
	ifconfig wlan0 up
	for item in $(iwlist wlan0 scan |grep SSID); do
	item=`echo $item|awk -F: '{print $2}'`
	ITEM_LIST="$ITEM_LIST,$item"
	done
	infow2 11 2 WIFI "OK(USB,$ITEM_LIST)"
else
	infow2 11 1 WIFI "Fail(USB,$ITEM_LIST)"
fi

#logw "$ITEM_LIST"

sleep 10
ITEM_LIST=""

fi



done




