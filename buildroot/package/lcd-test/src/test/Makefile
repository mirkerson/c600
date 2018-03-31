#LINUX_DIR=/home/danling/aw1623_fpga_0.0/kernel/linux-v2.6.36.4/include
CC := arm-none-linux-gnueabi-gcc
CFLAGS := -static  
CFLAGS += -O0

MAIN_SRC :=                    \
    dt_out.c\
    dt_capture.c\
    dt_test.c\
    dt_two_screen.c\
    dt_eh.c\


MAIN_SRC := $(strip $(MAIN_SRC))
MAIN_OBJ := $(addsuffix .o, $(basename $(MAIN_SRC)))
dep      := $(addsuffix .d, $(basename $(MAIN_SRC)))
EXE      := $(basename $(MAIN_SRC))


TARGET := $(EXE)

.PHONY: all
all: $(TARGET) clear

$(TARGET): %: %.o $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
	chmod a+x $@
clear:
	rm -rf *.o


.PHONY: clean
clean: 
	$(RM) $(OBJ) $(MAIN_OBJ) $(TARGET)  *~ 

