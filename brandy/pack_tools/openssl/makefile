
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
PLATFORM_LIBS += -Lopenssl_libs/ -lssl -lcrypto

OBJS =

REQ_MAIN  := req_main/libmain.o
X509_MAIN := x509_main/libmain.o
GENRSA_MAIN := genrsa_main/libmain.o
RSA_MAIN := rsa_main/libmain.o

LIBS  = $(COMMONDIR)/common/libcommon.o
LIBS += main/libapp.o

LIBS := $(addprefix $(obj),$(sort $(LIBS)))
.PHONY : $(LIBS) $(REQ_MAIN) $(X509_MAIN) $(GENRSA_MAIN) $(RSA_MAIN)

ALL-y += $(obj)req $(obj)x509 $(obj)genrsa $(obj)rsa

all: $(ALL-y)

$(obj)req:  $(LIBS) $(REQ_MAIN)
	$(CC) $(LIBS) $(REQ_MAIN) $(PLATFORM_LIBS) $(LDFLAGS) -o req
	@cp -v req $(TOPDIR)/../../../tools/pack/pctools/linux/openssl/req

$(REQ_MAIN):
	@$(MAKE) -C $(dir $(subst $(obj),,$@))

$(obj)x509: $(LIBS) $(X509_MAIN)
	$(CC) $(LIBS) $(X509_MAIN) $(PLATFORM_LIBS) $(LDFLAGS) -o x509
	@cp -v x509 $(TOPDIR)/../../../tools/pack/pctools/linux/openssl/x509

$(X509_MAIN):
	@$(MAKE) -C $(dir $(subst $(obj),,$@))

$(obj)genrsa: $(LIBS) $(GENRSA_MAIN)
	$(CC) $(LIBS) $(GENRSA_MAIN) $(PLATFORM_LIBS) $(LDFLAGS) -o genrsa
	@cp -v genrsa $(TOPDIR)/../../../tools/pack/pctools/linux/openssl/genrsa

$(GENRSA_MAIN):
	@$(MAKE) -C $(dir $(subst $(obj),,$@))

$(obj)rsa: $(LIBS) $(RSA_MAIN)
	$(CC) $(LIBS) $(RSA_MAIN) $(PLATFORM_LIBS) $(LDFLAGS) -o rsa
	@cp -v rsa $(TOPDIR)/../../../tools/pack/pctools/linux/openssl/rsa

$(RSA_MAIN):
	@$(MAKE) -C $(dir $(subst $(obj),,$@))

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
