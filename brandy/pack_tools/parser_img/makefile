

objects = parser_img.o 		\
		  firmware/imgdecode.o


edit:$(objects)
	gcc -o parser_img $(objects)
	rm -rf $(objects)


clean:
	rm -rf parser_img $(objects)