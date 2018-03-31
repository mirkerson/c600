#!/bin/sh

################################################################
#
# Just for test. Change this if you want to use it
#
#################################################################

ifconfig eth0 hw ether 3a:9d:33:28:d2:22
ifconfig eth0 192.168.3.122
ifconfig lo 127.0.0.1

mkdir /mnt/network
mount -t cifs -o user=benn,passwd=12345678 //192.168.3.47/benn /mnt/network

/etc/init.d/S50sshd start

echo "Add a new user"
adduser -s /bin/bash benn

