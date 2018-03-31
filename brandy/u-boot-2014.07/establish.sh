
make distclean
make sunivw1p1_config
make boot0

sleep 1

make distclean
make sunivw1p1_nor_config
make -j8

sleep 1

make distclean
make sunivw1p1_config
make -j8
