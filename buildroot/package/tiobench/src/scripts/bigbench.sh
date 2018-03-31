#!/bin/sh
# runs bonnie and tiobench.pl until you're tired of them
# be smart: boot with mem=16M and init 1 before running this
# if you want half-decent numbers

device=/dev/md3
half_one=/dev/sda10
half_two=/dev/sdb10
suffix=14-readbalance
outputdir=./results

# this is where we're mounting the raid test filesystem
benchdir=/bench

# size of the testfiles for both bonnie and tiotest
size=512
numruns=2

# time we need to sleep before resync finishes - empirical?
snooze=5m

bonnie=/usr/src/bonnie/Bonnie
tiobench=/usr/src/tiotest-0.24/tiobench.pl

# sekrit ninja crypto
flag=`echo --qd0kkx-enqbd | tr 0a-z a-z`

function doit()
{
	sed -e s/@@CHUNK/${1}k/ < raidtab.tp > raidtab
	mkraid --configfile raidtab $flag $device
	stride=`expr $1 / $2`
	block=`expr $2 \* 1024`
	echo Stride is $stride and Block is $block
	sync
	mke2fs -b $block -R stride=$stride $device
	sleep $snooze	# so the raid1 can sync in peace
	sync
	cat /proc/mdstat
	sync
	mount $device $benchdir
	echo Chunk is ${1}k, Stride is $stride and Block is ${block}k >> ${outputdir}/bonnie.${suffix}
	echo Chunk is ${1}k, Stride is $stride and Block is ${block}k >> ${outputdir}/tiobench.${suffix}
	$bonnie -s $size -d /bench >> ${outputdir}/bonnie.${suffix}
	$tiobench --dir /bench --size $size --numruns $numruns >> ${outputdir}/tiobench.${suffix}
}

function close() 
{
	umount $device
	umount $benchdir
	raidstop $device
}

mkdir -p ${outputdir}

cat << __EOF__ > raidtab.tp
raiddev		    $device
raid-level		    1
nr-raid-disks		    2
chunk-size		    @@CHUNK
persistent-superblock	    1
    device	    $half_one
    raid-disk     0
    device	    $half_two
    raid-disk     1

__EOF__

for f in 4 8 16 32 64 128 256; do
	for h in 1 2 4 ; do
		echo Benchmarking with ${f}k chunksize and ${h}k blocksize
		close
		doit $f $h
	done
done
close
rm raidtab.tp
rm raidtab
