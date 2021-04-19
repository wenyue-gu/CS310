OBJDIR := obj

# this port needs to be unique
GDBPORT	:= 25000

CC	:= gcc -pipe
GDB	:= gdb
AS	:= as
AR	:= ar
LD	:= ld
OBJCOPY	:= objcopy
OBJDUMP	:= objdump
NM	:= nm
QEMU    := qemu-system-i386

# For simpliciy (and to prevent subtle bugs) we'll assume that all c
# files depend on the headers in inc 

INC_HEADERS := $(wildcard inc/*.h)

# Compiler flags
# -fno-builtin is required to avoid refs to undefined functions in the kernel.
# Only optimize to -O1 to discourage inlining, which complicates backtraces.
CFLAGS := $(CFLAGS) $(DEFS) $(LABDEFS) -O1 -fno-builtin -I. -MD
CFLAGS += -fno-omit-frame-pointer
CFLAGS += -std=gnu99
CFLAGS += -static
CFLAGS += -Wall -Wno-format -Wno-unused -Werror -gstabs -m32
# -fno-tree-ch prevented gcc from sometimes reordering read_ebp() before
# mon_backtrace()'s function prologue on gcc version: (Debian 4.7.2-5) 4.7.2
CFLAGS += -fno-tree-ch

# Add -fno-stack-protector if the option exists.
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

# Common linker flags
LDFLAGS := -m elf_i386


GCC_LIB := $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

# Lists that the */Makefrag makefile fragments will add to
OBJDIRS :=

# Make sure that 'all' is the first target
all:

# Eliminate default suffix rules
.SUFFIXES:

# Delete target files if there is an error (or make is interrupted)
.DELETE_ON_ERROR:


# Include Makefrags for subdirectories
include boot/Makefrag
include kern/Makefrag
include lib/Makefrag
include user/Makefrag


QEMUOPTS = -drive file=$(OBJDIR)/kern/kernel.img,index=0,media=disk,format=raw -gdb tcp::$(GDBPORT)
QEMUOPTS += $(shell if $(QEMU) -nographic -help | grep -q '^-D '; then echo '-D qemu.log'; fi)
IMAGES = $(OBJDIR)/kern/kernel.img
QEMUOPTS += $(QEMUEXTRA)

.gdbinit: .gdbinit.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

gdb:
	$(GDB) -n -x .gdbinit

pre-qemu: .gdbinit

qemu: $(IMAGES) pre-qemu
	$(QEMU) -serial mon:stdio $(QEMUOPTS)

qemu-curses: $(IMAGES) pre-qemu
	$(QEMU) -serial null -serial mon:stdio -curses $(QEMUOPTS)

qemu-nox: $(IMAGES) pre-qemu
	@echo "***"
	@echo "*** Use Ctrl-a x to exit qemu"
	@echo "***"
	$(QEMU) -nographic $(QEMUOPTS)

qemu-gdb: $(IMAGES) pre-qemu
	@echo "***"
	@echo "*** Now run 'make gdb'." 1>&2
	@echo "***"
	$(QEMU) -serial mon:stdio $(QEMUOPTS) -S

qemu-curses-gdb: $(IMAGES) pre-qemu
	@echo "***"
	@echo "*** Now run 'make gdb'." 1>&2
	@echo "***"
	$(QEMU) -serial null -serial mon:stdio -curses $(QEMUOPTS) -S

qemu-nox-gdb: $(IMAGES) pre-qemu
	@echo "***"
	@echo "*** Now run 'make gdb'." 1>&2
	@echo "***"
	$(QEMU) -nographic $(QEMUOPTS) -S

print-qemu:
	@echo $(QEMU)

print-gdbport:
	@echo $(GDBPORT)

# For deleting the build
clean:
	rm -rf $(OBJDIR) .gdbinit jos.in qemu.log

realclean: clean
	rm -rf lab$(LAB).tar.gz \
		jos.out $(wildcard jos.out.*) \
		qemu.pcap $(wildcard qemu.pcap.*) \
		myapi.key

distclean: realclean
	rm -rf conf/gcc.mk

ifneq ($(V),@)
GRADEFLAGS += -v
endif

# For test runs
QEMUOPTS_TEST = -gdb tcp::$(GDBPORT)
QEMUOPTS_TEST += $(shell if $(QEMU) -nographic -help | grep -q '^-D '; then echo '-D qemu.log'; fi)
QEMUOPTS_TEST += $(QEMUEXTRA)

run-%-nox-gdb: $(OBJDIR)/kern/%.img pre-qemu
	$(QEMU) -nographic -drive file=$(OBJDIR)/kern/$*.img,index=0,media=disk,format=raw $(QEMUOPTS_TEST) -S

run-%-curses-gdb: $(OBJDIR)/kern/%.img pre-qemu
	$(QEMU) -serial null -serial mon:stdio -curses -drive file=$(OBJDIR)/kern/$*.img,index=0,media=disk,format=raw $(QEMUOPTS_TEST) -S

run-%-gdb: $(OBJDIR)/kern/%.img pre-qemu
	$(QEMU) -serial mon:stdio -drive file=$(OBJDIR)/kern/$*.img,index=0,media=disk,format=raw $(QEMUOPTS_TEST) -S

run-%-nox: $(OBJDIR)/kern/%.img pre-qemu
	$(QEMU) -nographic -drive file=$(OBJDIR)/kern/$*.img,index=0,media=disk,format=raw $(QEMUOPTS_TEST)

run-%-curses: $(OBJDIR)/kern/%.img pre-qemu
	$(QEMU) -serial null -serial mon:stdio -curses -drive file=$(OBJDIR)/kern/$*.img,index=0,media=disk,format=raw $(QEMUOPTS_TEST)

run-%: $(OBJDIR)/kern/%.img pre-qemu
	$(QEMU) -serial mon:stdio -drive file=$(OBJDIR)/kern/$*.img,index=0,media=disk,format=raw $(QEMUOPTS_TEST)

always:
	@:

.PHONY: all always \
	handin git-handin tarball tarball-pref clean realclean distclean grade handin-prep handin-check
