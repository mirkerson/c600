

objects = sunxi_sign.o crc.o openssl.o common.o


edit:$(objects)
	gcc -o sunxi_sign $(objects) -lssl -lcrypto
	rm -rf $(objects)


clean:
	rm -rf sunxi_sign $(objects)