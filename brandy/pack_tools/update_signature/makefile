

objects = update_signature.o rsa.o hash.o crc.o


edit:$(objects)
	gcc -o signature $(objects)
	rm -rf $(objects)


clean:
	rm -rf update_signature $(objects)