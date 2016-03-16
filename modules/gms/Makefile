# This file contains build instructions.
#
#

MODULE = gms

SRCS = mode.c drop.c flags.c saregister.c sajoin.c signal.c part.c join.c cloak.c invite.c set.c show.c info.c list.c register.c home.c help.c gms.c test.c

include ../../extra.mk
include ../../buildsys.mk
include ../../buildsys.module.mk

CPPFLAGS += -I../../include -fgnu89-inline -fplan9-extensions -fmax-errors=1 -O0 -fms-extensions
LIBS +=	-L../../libathemecore -lathemecore ${LDFLAGS_RPATH}

