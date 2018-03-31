
##
## Makefile for Sunxi Secure Boot
##


MAJOR = 2014
MINOR = 05
SUBLEVEL =
EXTRAVERSION = -rc1
SECURE_BOOT_VERSION = $(MAJOR).$(MINOR).$(SUBLEVEL)$(EXTRAVERSION)

OBJTREE		:= $(CURDIR)
SRCTREE		:= $(CURDIR)
TOPDIR		:= $(SRCTREE)
LNDIR		:= $(OBJTREE)
SRCTREE		:= $(CURDIR)
TOPDIR		:= $(SRCTREE)
COMMONDIR   := $(CURDIR)/..

export	TOPDIR SRCTREE OBJTREE TOOLSDIR COMMONDIR

ifneq ($(OBJTREE),$(SRCTREE))
obj := $(OBJTREE)/
src := $(SRCTREE)/
else
obj :=
src :=
endif
export obj src

include $(TOPDIR)/config.mk

#
# We let the C compiler driver to take care of .s files. This is done in
# order to be excused from maintaining a separate set of architecture
# dependent assembler flags.
#

OBJS =

LIBS  = main/libmain.o
LIBS += certif/libcertif.o
LIBS += config/libconfig.o
LIBS += hash/libhash.o
LIBS += key/libkey.o
LIBS += toc/libtoc.o
LIBS += rsa/librsa.o
LIBS += $(COMMONDIR)/common/libcommon.o



PLATFORM_LIBS = -lstdc++

LIBS := $(addprefix $(obj),$(sort $(LIBS)))
.PHONY : $(LIBS)

ALL-y += $(obj)dragonsecboot

all: $(ALL-y)

$(obj)dragonsecboot:  $(LIBS)
	$(CC) $(LIBS) $(LDFLAGS) $(PLATFORM_LIBS) -o dragonsecboot
	@cp -v dragonsecboot $(TOPDIR)/../../../tools/pack/pctools/linux/openssl/dragonsecboot

$(LIBS):
	@$(MAKE) -C $(dir $(subst $(obj),,$@))

clean:
	@find $(OBJTREE) $(COMMONDIR)/common/ -type f \
		\( -name '*.depend' -o 	\
		   -name 'core'     -o 	\
		   -name '*.bak'    -o 	\
		   -name '*~'       -o  \
		   -name '*.o'	    -o  \
		   -name '*.exe'	\) -print \
		| xargs rm -f

# DO NOT DELETE THIS LINE -- make depend depends on it.
