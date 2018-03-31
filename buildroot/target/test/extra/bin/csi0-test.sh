#!/bin/sh

while true;


do

CSI_OUT=""
COLOR=""
	csi0_read
	RESULT=$?
	if [ $RESULT -eq 0 ]; then
		CSI_OUT="csi0 OK"
		COLOR="2"
	else
		CSI_OUT="csi0 Fail"
		COLOR="1"
	fi

infow2 13 $COLOR CSI "$CSI_OUT"

sleep 1


done






