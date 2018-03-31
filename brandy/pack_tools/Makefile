all:
	make -C create_mbr
	make -C update_boot0
	make -C update_checksum
	make -C update_fes1
	make -C update_signature
	make -C update_simg
	make -C update_uboot
	make -C parser_img
	make -C openssl_signature
	make -C script

	cp create_mbr/update_mbr 				../../tools/pack/pctools/linux/mod_update/update_mbr
	cp update_boot0/update_boot0 			../../tools/pack/pctools/linux/mod_update/update_boot0
	cp update_checksum/genchecksum_uboot 	../../tools/pack/pctools/linux/mod_update/genchecksum_uboot
	cp update_fes1/update_fes1  			../../tools/pack/pctools/linux/mod_update/update_fes1
	cp update_signature/signature 			../../tools/pack/pctools/linux/mod_update/signature
	cp update_simg/simg 					../../tools/pack/pctools/linux/mod_update/simg
	cp update_uboot/update_uboot 			../../tools/pack/pctools/linux/mod_update/update_uboot
	cp parser_img/parser_img 			    ../../tools/pack/pctools/linux/mod_update/parser_img
	cp openssl_signature/sunxi_sign 		../../tools/pack/pctools/linux/mod_update/sunxi_sign
	cp script/script 		../../tools/pack/pctools/linux/mod_update/script


