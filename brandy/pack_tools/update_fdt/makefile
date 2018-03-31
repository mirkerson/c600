

objects = update_fdt.o


edit:$(objects)
	gcc -o update_uboot_fdt $(objects) -static
	rm -rf $(objects)

	cp update_uboot_fdt  	../../../tools/pack/pctools/linux/mod_update/

clean:
	rm -rf update_fdt $(objects)
