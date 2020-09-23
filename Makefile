K=kernel

OBJS = $K/entry.o 				\
		$K/printf.o				\
		$K/interrupt.o			\
		$K/trap.o				\
		$K/timer.o				\
		$K/memory.o				\
		$K/paging.o				\
		$K/heap.o				\
		$K/main.o

# 生成工具链前缀
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

LDFLAGS = -z max-page-size=4096

QEMU = qemu-system-riscv64
QEMUFLAGS = -machine virt -bios default -device loader,file=Image,addr=0x80200000 -nographic

all: Image

Image: $(subst .c,.o,$(wildcard $K/*.c)) $(subst .S,.o,$(wildcard $K/*.S))
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $K/Image $(OBJS)
	$(OBJCOPY) $K/Image --strip-all -O binary $@


$K/%.o: $K/%.c $K/%.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f */*.d */*.o $K/Image Image Image.asm

asm: Image
	$(OBJDUMP) -S $K/Image > Image.asm

qemu: Image
	$(QEMU) $(QEMUFLAGS)
