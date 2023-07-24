B=bootloader
K=kernel
U=user
T=tools

HDRS = \
	$K/asm.h\
	$K/buf.h\
	$K/date.h\
	$K/defs.h\
	$K/elf.h\
	$K/fcntl.h\
	$K/file.h\
	$K/fs.h\
	$K/kbd.h\
	$K/memlayout.h\
	$K/mmu.h\
	$K/mp.h\
	$K/param.h\
	$K/proc.h\
	$K/sleeplock.h\
	$K/spinlock.h\
	$K/stat.h\
	$K/syscall.h\
	$K/traps.h\
	$K/types.h\
	$K/x86.h\
	$U/user.h\

OBJS = \
	$K/bio.o\
	$K/console.o\
	$K/exec.o\
	$K/file.o\
	$K/fs.o\
	$K/ide.o\
	$K/ioapic.o\
	$K/kalloc.o\
	$K/kbd.o\
	$K/lapic.o\
	$K/log.o\
	$K/main.o\
	$K/mp.o\
	$K/picirq.o\
	$K/pipe.o\
	$K/proc.o\
	$K/sleeplock.o\
	$K/spinlock.o\
	$K/string.o\
	$K/swtch.o\
	$K/syscall.o\
	$K/sysfile.o\
	$K/sysproc.o\
	$K/trapasm.o\
	$K/trap.o\
	$K/uart.o\
	$K/vectors.o\
	$K/vm.o\

# Cross-compiling (e.g., on Mac OS X)
# TOOLPREFIX = i386-jos-elf

# Using native tools (e.g., on X86 Linux)
#TOOLPREFIX = 

# Try to infer the correct TOOLPREFIX if not set
ifndef TOOLPREFIX
TOOLPREFIX := $(shell if i386-jos-elf-objdump -i 2>&1 | grep '^elf32-i386$$' >/dev/null 2>&1; \
	then echo 'i386-jos-elf-'; \
	elif objdump -i 2>&1 | grep 'elf32-i386' >/dev/null 2>&1; \
	then echo ''; \
	else echo "***" 1>&2; \
	echo "*** Error: Couldn't find an i386-*-elf version of GCC/binutils." 1>&2; \
	echo "*** Is the directory with i386-jos-elf-gcc in your PATH?" 1>&2; \
	echo "*** If your i386-*-elf toolchain is installed with a command" 1>&2; \
	echo "*** prefix other than 'i386-jos-elf-', set your TOOLPREFIX" 1>&2; \
	echo "*** environment variable to that prefix and run 'make' again." 1>&2; \
	echo "*** To turn off this error, run 'gmake TOOLPREFIX= ...'." 1>&2; \
	echo "***" 1>&2; exit 1; fi)
endif

# If the makefile can't find QEMU, specify its path here
# QEMU = qemu-system-i386

# Try to infer the correct QEMU
ifndef QEMU
QEMU = $(shell if which qemu > /dev/null; \
	then echo qemu; exit; \
	elif which qemu-system-i386 > /dev/null; \
	then echo qemu-system-i386; exit; \
	elif which qemu-system-x86_64 > /dev/null; \
	then echo qemu-system-x86_64; exit; \
	else \
	qemu=/Applications/Q.app/Contents/MacOS/i386-softmmu.app/Contents/MacOS/i386-softmmu; \
	if test -x $$qemu; then echo $$qemu; exit; fi; fi; \
	echo "***" 1>&2; \
	echo "*** Error: Couldn't find a working QEMU executable." 1>&2; \
	echo "*** Is the directory containing the qemu binary in your PATH" 1>&2; \
	echo "*** or have you tried setting the QEMU variable in Makefile?" 1>&2; \
	echo "***" 1>&2; exit 1)
endif

CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump
CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -ggdb -m32 -fno-omit-frame-pointer -I.
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
ASFLAGS = -m32 -I. -gdwarf-2 -Wa,-divide
# FreeBSD ld wants ``elf_i386_fbsd''
LDFLAGS += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

# Disable PIE when possible (for Ubuntu 16.10 toolchain)
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]no-pie'),)
CFLAGS += -fno-pie -no-pie
endif
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]nopie'),)
CFLAGS += -fno-pie -nopie
endif

all: xv6.img fs.img

# Ensure that any header changes cause all sources to be recompiled.
%.o: %.c $(HDRS)
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.S $(HDRS)
	$(CC) -c $(ASFLAGS) -o $@ $<

xv6.img: $B/bootblock $K/kernel
	dd if=/dev/zero of=xv6.img count=10000
	dd if=$B/bootblock of=xv6.img conv=notrunc
	dd if=$K/kernel of=xv6.img seek=1 conv=notrunc

xv6memfs.img: $B/bootblock $K/kernelmemfs
	dd if=/dev/zero of=xv6memfs.img count=10000
	dd if=$B/bootblock of=xv6memfs.img conv=notrunc
	dd if=$K/kernelmemfs of=xv6memfs.img seek=1 conv=notrunc

$B/bootblock: $B/bootasm.S $B/bootmain.c
	$(CC) $(CFLAGS) -fno-pic -O -nostdinc -I. -c $B/bootmain.c -o $B/bootmain.o
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I. -c $B/bootasm.S -o $B/bootasm.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o $B/bootblock.o $B/bootasm.o $B/bootmain.o
	$(OBJCOPY) -S -O binary -j .text $B/bootblock.o $B/bootblock
	$T/sign.pl $B/bootblock

$K/entryother: $K/entryother.S
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I. -c $K/entryother.S -o $K/entryother.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7000 -o $B/bootblockother.o $K/entryother.o
	$(OBJCOPY) -S -O binary -j .text $B/bootblockother.o $K/entryother

$U/initcode: $U/initcode.S
	$(CC) $(CFLAGS) -nostdinc -I. -c $U/initcode.S -o $U/initcode.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o $U/initcode.out $U/initcode.o
	$(OBJCOPY) -S -O binary $U/initcode.out $U/initcode

$K/kernel: $(OBJS) $K/entry.o $K/entryother $U/initcode $K/kernel.ld
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $K/kernel $K/entry.o $(OBJS) -b binary $U/initcode $K/entryother

# kernelmemfs is a copy of kernel that maintains the
# disk image in memory instead of writing to a disk.
# This is not so useful for testing persistent storage or
# exploring disk buffering implementations, but it is
# great for testing the kernel on real hardware without
# needing a scratch disk.
MEMFSOBJS = $(filter-out $K/ide.o,$(OBJS)) $K/memide.o
$K/kernelmemfs: $(MEMFSOBJS) $K/entry.o $K/entryother $U/initcode $K/kernel.ld fs.img
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $K/kernelmemfs $K/entry.o  $(MEMFSOBJS) -b binary $U/initcode $K/entryother fs.img

tags: $(OBJS) $K/entryother.S $U/_init
	etags */*.S */*.c

$K/vectors.S: $T/vectors.pl
	$T/vectors.pl > $K/vectors.S

ULIB = $U/ulib.o $U/usys.o $U/printf.o $U/umalloc.o

_%: %.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^

$U/_forktest: $U/forktest.o $(ULIB)
	# forktest has less library code linked in - needs to be small
	# in order to be able to max out the proc table.
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $U/_forktest $U/forktest.o $U/ulib.o $U/usys.o

$T/mkfs: $T/mkfs.c $K/fs.h
	gcc -Wall -I. -o $T/mkfs $T/mkfs.c

# Prevent deletion of intermediate files, e.g. cat.o, after first build, so
# that disk image changes after first build are persistent until clean.  More
# details:
# http://www.gnu.org/software/make/manual/html_node/Chained-Rules.html
.PRECIOUS: %.o

UPROGS=\
	$U/_cat\
	$U/_echo\
	$U/_forktest\
	$U/_grep\
	$U/_init\
	$U/_kill\
	$U/_ln\
	$U/_ls\
	$U/_mkdir\
	$U/_rm\
	$U/_sh\
	$U/_stressfs\
	$U/_usertests\
	$U/_wc\
	$U/_zombie\
	$U/_blockwriter\
	$U/_setkey\
	$U/_encr\
	$U/_decr\

fs.img: $T/mkfs README $(UPROGS)
	$T/mkfs fs.img README $(UPROGS)

clean: 
	rm -f *.tex *.dvi *.idx *.aux *.log *.ind *.ilg \
	*/*.o */*.d */*.asm */*.sym $K/vectors.S $B/bootblock $K/entryother \
	$U/initcode $U/initcode.out $K/kernel xv6.img fs.img $K/kernelmemfs \
	xv6memfs.img $T/mkfs .gdbinit \
	$(UPROGS)

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)
ifndef CPUS
CPUS := 2
endif
QEMUOPTS = -drive file=fs.img,index=1,media=disk,format=raw -drive file=xv6.img,index=0,media=disk,format=raw -smp $(CPUS) -m 512 $(QEMUEXTRA)

qemu: fs.img xv6.img
	$(QEMU) $(QEMUOPTS)

qemu-memfs: xv6memfs.img
	$(QEMU) -drive file=xv6memfs.img,index=0,media=disk,format=raw -smp $(CPUS) -m 256

qemu-nox: fs.img xv6.img
	$(QEMU) -serial mon:stdio -nographic $(QEMUOPTS)

.gdbinit: .gdbinit.tmpl
	sed "s/localhost:1234/localhost:$(GDBPORT)/" < $^ > $@

qemu-gdb: fs.img xv6.img .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB)

qemu-nox-gdb: fs.img xv6.img .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -serial mon:stdio -nographic $(QEMUOPTS) -S $(QEMUGDB)
