K=kernel

OBJS = 						\
	$K/entry.o				\
	$K/trap.o				\
	$K/printf.o				\
	$K/interrupt.o			\
	$K/timer.o				\
	$K/memory.o				\
	$K/paging.o				\
	$K/heap.o				\
	$K/process.o			\
	$K/scheduler.o			\
	$K/main.o

# Try to infer the correct TOOLPREFIX if not set
ifndef TOOLPREFIX
TOOLPREFIX := $(shell if riscv64-unknown-elf-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-unknown-elf-'; \
	elif riscv64-linux-gnu-objdump -i 2>&1 | grep 'elf64-big' >/dev/null 2>&1; \
	then echo 'riscv64-linux-gnu-'; \
	else echo "***" 1>&2; \
	echo "*** Error: Couldn't find an riscv64 version of GCC/binutils." 1>&2; \
	echo "*** To turn off this error, run 'gmake TOOLPREFIX= ...'." 1>&2; \
	echo "***" 1>&2; exit 1; fi)
endif

QEMU = qemu-system-riscv64
QEMUOPTS = -machine virt -bios default -device loader,file=Image,addr=0x80200000 --nographic

CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -I.
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

# Disable PIE when possible (for Ubuntu 16.10 toolchain)
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]no-pie'),)
CFLAGS += -fno-pie -no-pie
endif
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]nopie'),)
CFLAGS += -fno-pie -nopie
endif

LDFLAGS = -z max-page-size=4096

all: Image

Image: $(subst .c,.o,$(wildcard $K/*.c)) $(subst .S,.o,$(wildcard $K/*.S))
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $K/kernel $(OBJS)
	$(OBJCOPY) $K/kernel --strip-all -O binary $@

# compile all .c and .S file to .o file
$K/%.o: $K/%.c $K/%.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f */*.d */*.o $K/kernel Image Image.asm

asm: Image
	$(OBJDUMP) -S $K/kernel > $K/kernel.asm

qemu: Image
	$(QEMU) $(QEMUOPTS)

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)

qemu-gdb: Image
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB)