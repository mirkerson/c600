

objects = check.o script.o update_checksum.o


edit:$(objects)
	gcc -o genchecksum_uboot $(objects)
	rm -rf $(objects)


clean:
	rm -rf genchecksum_uboot $(objects)