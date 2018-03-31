#!/bin/bash


BR_ROOT=`(cd ../../; pwd)`

export PATH=$PATH:$BR_ROOT/target/tools/host/usr/bin

rm -rf skel/
mkdir skel
gzip -dc skel.cpio.gz |(cd skel/; cpio -i)

cp -rfv extra/* skel/

NR_SIZE=`du -s skel|awk '{print $1}'`
NEW_NR_SIZE=0
TARGET_IMAGE=system.ext4

((NEW_NR_SIZE=$NR_SIZE*12/10))


echo "blocks: $NR_SIZE -> $NEW_NR_SIZE"
genext2fs -d skel -b $NEW_NR_SIZE $TARGET_IMAGE

tune2fs -j -O extents,uninit_bg,dir_index $TARGET_IMAGE

fsck.ext4 -y $TARGET_IMAGE



