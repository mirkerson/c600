
##
## Makefile for Sunxi Secure Boot
##



include $(TOPDIR)/config.mk

LIB	:= $(obj)librsa.o

COBJS-y += BigInt.o
COBJS-y += rsa.o

SRCS	:= $(COBJS-y:.o=.cpp)
OBJS	:= $(addprefix $(obj),$(COBJS-y))

all:	 $(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
