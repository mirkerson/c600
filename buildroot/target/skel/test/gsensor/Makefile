# Makefile for alsa test routine

SRC = mxc622x_test.c
BIN = mxc622x_test

CC = arm-none-linux-gnueabi-gcc

all:
	$(CC) $(SRC) ${LIB} -static -o $(BIN)

clean:
	rm -f $(BIN)
