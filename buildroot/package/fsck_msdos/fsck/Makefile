
SRCS := boot.c check.c dir.c dosfs.h ext.h fat.c fsutil.h main.c 
CFLAGS := -Wall -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -I. -lc

fsck_msdos : $(SRCS)
	$(CC) $(filter %.c, $(SRCS)) $(CFLAGS) -o fsck_msdos
clean:	
	rm fsck_msdos
install:
	cp fsck_msdos $(DESTDIR)/usr/bin/
