.SUFFIXES:

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
AR := $(CROSS_COMPILE)ar

EXTRA_DEFS := -D_FILE_OFFSET_BITS=64 -DHAVE_ACCEPT4=1
CFLAGS := -g -pipe -O2 -Wall \
	-Wsign-compare -Wcast-align \
	-Waggregate-return \
	-Wstrict-prototypes \
	-Wmissing-prototypes \
	-Wmissing-declarations \
	-Wmissing-noreturn \
	-finline-functions \
	-Wmissing-format-attribute \
	-Wno-cast-align \
	-fwrapv \
	-Iinclude \
	$(EXTRA_DEFS) 

COMPRESS_BIN := compress
COMPRESS_LIBS := 
COMPRESS_OBJ = compress.o \
		hexdump.o \
		bwte.o \
		mtfe.o \
		os.o

DECOMPRESS_BIN := decompress
DECOMPRESS_LIBS := 
DECOMPRESS_OBJ = decompress.o \
		hexdump.o \
		mtfd.o \
		bwtd.o \
		os.o

ALL_BIN := $(COMPRESS_BIN) $(DECOMPRESS_BIN)
ALL_OBJ := $(COMPRESS_OBJ) $(DECOMPRESS_OBJ)
ALL_DEP := $(patsubst %.o, .%.d, $(ALL_OBJ))
ALL_TARGETS := $(ALL_BIN)

TARGET: all

.PHONY: all clean walk

all: $(ALL_BIN)

ifeq ($(filter clean, $(MAKECMDGOALS)),clean)
CLEAN_DEP := clean
else
CLEAN_DEP :=
endif

%.o %.d: %.c $(CLEAN_DEP) $(ROOT_DEP) $(CONFIG_MAK) Makefile
	@echo " [C] $<"
	@$(CC) $(CFLAGS) -MMD -MF $(patsubst %.o, .%.d, $@) \
		-MT $(patsubst .%.d, %.o, $@) \
		-c -o $(patsubst .%.d, %.o, $@) $<

$(COMPRESS_BIN): $(COMPRESS_OBJ)
	@echo " [LINK] $@"
	@$(CC) $(CFLAGS) -o $@ $(COMPRESS_OBJ) $(COMPRESS_LIBS)

$(DECOMPRESS_BIN): $(DECOMPRESS_OBJ)
	@echo " [LINK] $@"
	@$(CC) $(CFLAGS) -o $@ $(DECOMPRESS_OBJ) $(DECOMPRESS_LIBS)

clean:
	rm -f $(ALL_TARGETS) $(ALL_OBJ) $(ALL_DEP)

ifneq ($(MAKECMDGOALS),clean)
-include $(ALL_DEP)
endif
