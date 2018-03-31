#!/bin/sh

cp ${LICHEE_BR_OUT}/external-toolchain/arm-linux-gnueabi/libc/lib/arm-linux-gnueabi/* ${LICHEE_BR_OUT}/target/lib/ -pfr 
chmod +x ${LICHEE_BR_OUT}/target/lib/*
rm -rf ${LICHEE_BR_OUT}/target/init
(cd ${LICHEE_BR_OUT}/target && ln -s bin/busybox init)

cat > ${LICHEE_BR_OUT}/target/etc/init.d/rcS << EOF
#!/bin/sh

mount -t devtmpfs none /dev
mkdir /dev/pts
mount -t devpts none /dev/pts
mount -t sysfs sysfs /sys
mknod /dev/mali c 230 0
hostname sun6i 
mkdir -p /boot
mount /dev/nanda /boot
MODULES_DIR=/lib/modules/\`uname -r\`


EOF

sed -i '/TSLIB/d' ${LICHEE_BR_OUT}/target/etc/profile

echo "export TSLIB_TSEVENTTYPE=H3600" >> ${LICHEE_BR_OUT}/target/etc/profile
echo "export TSLIB_CONSOLEDEVICE=none" >> ${LICHEE_BR_OUT}/target/etc/profile
echo "export TSLIB_FBDEVICE=/dev/fb0" >> ${LICHEE_BR_OUT}/target/etc/profile
echo "export TSLIB_TSDEVICE=/dev/input/event2" >> ${LICHEE_BR_OUT}/target/etc/profile
echo "export TSLIB_CALIBFILE=/etc/pointercal" >> ${LICHEE_BR_OUT}/target/etc/profile
echo "export TSLIB_CONFFILE=/etc/ts.conf" >> ${LICHEE_BR_OUT}/target/etc/profile
echo "export TSLIB_PLUGINDIR=/usr/lib/ts" >> ${LICHEE_BR_OUT}/target/etc/profile
echo "" >> ${LICHEE_BR_OUT}/target/etc/profile

touch ${LICHEE_BR_OUT}/target/etc/init.d/auto_config_network

cat > ${LICHEE_BR_OUT}/target/etc/init.d/auto_config_network << EOF
#!/bin/sh

MAC_ADDR="\`uuidgen |awk -F- '{print \$5}'|sed 's/../&:/g'|sed 's/\(.\)$//' |cut -b3-17\`"

ifconfig eth0 hw ether "48\$MAC_ADDR"
ifconfig lo 127.0.0.1
udhcpc

EOF

chmod +x ${LICHEE_BR_OUT}/target/etc/init.d/auto_config_network
(cd target/skel/ && tar -c *) |tar -C ${LICHEE_BR_OUT}/target/ -xv


