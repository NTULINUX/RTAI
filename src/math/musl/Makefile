# Makefile for musl (requires GNU make)
#
# Heavily modified by Alec Ari

srcdir = .

SRC_DIRS = ./libm/*
BASE_GLOBS = $(addsuffix .c,$(SRC_DIRS))
BASE_SRCS = $(sort $(wildcard $(BASE_GLOBS)))
BASE_OBJS = $(patsubst $(srcdir)/%,%.o,$(basename $(BASE_SRCS)))
ALL_OBJS = $(addprefix obj/, $(sort $(BASE_OBJS)))
LIBM_OBJS = $(filter obj/%,$(ALL_OBJS))

CFLAGS = -O2 -march=x86-64 -mcmodel=kernel -fno-common -fno-pic -pipe -fomit-frame-pointer -std=c99 -nostdinc -ffreestanding -fexcess-precision=standard -frounding-math
CFLAGS += -fno-unwind-tables -fno-asynchronous-unwind-tables -ffunction-sections -fdata-sections -Iobj/include -I$(srcdir)/include
CFLAGS += -D_XOPEN_SOURCE=700 -Wa,--noexecstack -Wall -Werror

LDFLAGS = -Wl,--sort-section,alignment -Wl,--sort-common -Wl,--gc-sections -Wl,--hash-style=both -Wl,--no-undefined -Wl,--exclude-libs=ALL

STATIC_LIBS = lib/libm.a

all: $(STATIC_LIBS)

OBJ_DIRS = $(sort $(patsubst %/,%,$(dir $(STATIC_LIBS) $(ALL_OBJS))))

$(STATIC_LIBS) $(ALL_OBJS) $(ALL_OBJS:%.o=%.lo): | $(OBJ_DIRS)

$(OBJ_DIRS):
	mkdir -p $@

obj/%.o: $(srcdir)/%.c
	gcc $(CFLAGS) -c -o $@ $<

lib/libm.a: $(LIBM_OBJS)
	rm -f $@
	ar rcT $@ $(LIBM_OBJS)
	ranlib $@

clean:
	rm -rf obj lib

.PHONY: all clean
