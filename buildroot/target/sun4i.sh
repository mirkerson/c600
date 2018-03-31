#!/bin/sh

#rm -rf output/target/etc/init.d/S*

rm -rf output/target/init
(cd output/target && ln -s bin/busybox init)


cat > output/target/etc/init.d/rcS << EOF
#!/bin/sh

mount -t devtmpfs none /dev
mkdir /dev/pts
mount -t devpts none /dev/pts

mknod /dev/mali c 230 0
hostname sun4i
mkdir -p /boot
mount /dev/nanda /boot
MODULES_DIR=/lib/modules/\`uname -r\`
(cd \$MODULES_DIR;insmod disp.ko;insmod lcd.ko;insmod hdmi.ko)


EOF

sed -i '/TSLIB/d' output/target/etc/profile

echo "export TSLIB_TSEVENTTYPE=H3600" >> output/target/etc/profile
echo "export TSLIB_CONSOLEDEVICE=none" >> output/target/etc/profile
echo "export TSLIB_FBDEVICE=/dev/fb0" >> output/target/etc/profile
echo "export TSLIB_TSDEVICE=/dev/input/event2" >> output/target/etc/profile
echo "export TSLIB_CALIBFILE=/etc/pointercal" >> output/target/etc/profile
echo "export TSLIB_CONFFILE=/etc/ts.conf" >> output/target/etc/profile
echo "export TSLIB_PLUGINDIR=/usr/lib/ts" >> output/target/etc/profile
echo "" >> output/target/etc/profile

touch output/target/etc/init.d/auto_config_network

cat > output/target/etc/init.d/auto_config_network << EOF
#!/bin/sh

MAC_ADDR="\`uuidgen |awk -F- '{print \$5}'|sed 's/../&:/g'|sed 's/\(.\)$//' |cut -b3-17\`"

ifconfig eth0 hw ether "48\$MAC_ADDR"
ifconfig lo 127.0.0.1
udhcpc

EOF

chmod +x output/target/etc/init.d/auto_config_network
(cd target/skel/ && tar -c *) |tar -C output/target/ -xv


