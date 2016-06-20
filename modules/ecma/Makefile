# Copyright (c) 2003-2004 E. Will et al.
# Rights to this code are documented in doc/LICENSE.
#
# This file contains build instructions.
#
#

MODULE = ecma

SRCS = $(shell find $(SOURCEDIR) -name '*.c')

include ../../extra.mk
include ../../buildsys.mk
include ../../buildsys.module.mk

CPPFLAGS += -I../../include -x c++ -std=gnu++14 -ggdb -Wall -Wextra -Wno-vla -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter -Wno-missing-field-initializers -O0
CPPFLAGS += -fmax-errors=1
#CPPFLAGS += -DDEBUG_INTERCEPTS
CPPFLAGS += -I../pgsql/include/
LIBS +=	-L../../libathemecore -lathemecore ${LDFLAGS_RPATH}

CPPFLAGS += -I/home/jason/v8
CPPFLAGS += -I/home/jason/v8/include/
LIBS += -lpq                                                           # pgsql (vol.c)
LIBS += -L/home/jason/v8/out/x64.debug/obj.target/tools/gyp/           # static
LIBS += -L/home/jason/v8/out/x64.debug/obj.target/third_party/icu      # static
LIBS += -L/home/jason/v8/out/x64.debug/lib.target/                     # shared
#LIBS += -lv8_base -lv8_libbase -lv8_nosnapshot -lv8_libplatform        # static
#LIBS += -lv8 -lv8_base -lv8_nosnapshot -lv8_libplatform                # -lv8 shared, rest static
LIBS += -lv8 -lv8_nosnapshot -lv8_libplatform                          # -lv8 shared, rest static
LIBS += -licuuc -licui18n -licudata                                    # either
LIBS += -lrt -ldl -pthread -lstdc++                                    # sys
