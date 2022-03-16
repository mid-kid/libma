HOSTCC ?= cc
AR = thumb-elf-ar
AS = thumb-elf-as
CC = thumb-elf-gcc

OFILES = ma_bios.o ma_api.o ma_sub.o md5c.o ma_ango.o ma_var.o
LIB = libma.a
AGBDIR ?= /agb

ASFLAGS = -I$(AGBDIR)/include -mthumb-interwork
CFLAGS = -O2 -Wall -I$(AGBDIR)/include -I. \
	-mthumb-interwork -nostdlib

.SUFFIXES: $(SUFFIXES) .dat
.dat.o:
	objcopy -I binary -O elf32-little $< $@
.c.s:
	$(CC) $(CFLAGS) -S $<

all: compare

$(LIB): $(OFILES) ardata ardata.txt
	$(AR) rs $@ $(OFILES)
	./ardata $@ ardata.txt

compare: $(LIB)
	md5sum -c libma.md5

clean:
	rm -f $(LIB) $(OFILES) ardata ardata.exe

ardata: CC := $(HOSTCC)
ardata: CFLAGS := $(HOSTCFLAGS)

# dependence
ma_bios.o:
ma_api.o:
ma_sub.o:
md5c.o:
ma_ango.o:
ma_var.o:
