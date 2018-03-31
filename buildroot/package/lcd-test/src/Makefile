CFLAGS += -static 
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

.PHONY: all install
all: $(TARGET)

$(TARGET): %: %.o $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
	chmod a+x $@
clear:
	rm -rf *.o


install:
	echo $(DESTDIR)
	echo "benn install"
	install dt_out $(DESTDIR)/bin
	install dt_capture $(DESTDIR)/bin
	install dt_test $(DESTDIR)/bin
	install dt_two_screen $(DESTDIR)/bin
	install dt_eh $(DESTDIR)/bin
	install pic/memin_shg.bin $(DESTDIR)/bin

.PHONY: clean
clean: 
	$(RM) $(OBJ) $(MAIN_OBJ) $(TARGET)  *~ 

