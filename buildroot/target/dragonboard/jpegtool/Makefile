
# define sources root directory before everything
SRC_ROOT = .

# change compiler and linker option before you include rule.mk
#
# link to libscript.a when you need to fetch configuration
# from test_script
#
#CFLAGS := $(CFLAGS) -g
LDFLAGS := -ljpeg

# include rule.mk
include $(SRC_ROOT)/rule.mk

CFLAGS := $(CFLAGS) 




	 
TARGET = jpegtool
OBJS = jpeg.o capture.o

all: $(TARGET)

$(TARGET):$(OBJS)
	$(LINK_MSG)
	$(LINKX)
	
$(OBJS): %.o: %.c
	$(COMPILE_MSG)
	$(COMPILEX)

.PHONY:clean

clean:
	rm *.o $(TARGET) -f
cleanobj:
	rm *.o 
