

objects = crc.o script.o update_mbr.o


edit:$(objects)
	gcc -o update_mbr $(objects)
	rm -rf $(objects)


clean:
	rm -rf update_mbr $(objects)