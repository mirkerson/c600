CFLAGS += -static 
CFLAGS += -O0

.PHONY: all install
all: input-test

input-test:
	$(CC) $(CFLAGS) -o input-test input-test.c

install:
	install input-test $(DESTDIR)/bin

.PHONY: clean

