#
#  Makefile
#  
#  (C) 2021  Ziyang Guo
#

K=kernel
U=user

OBJS = 						\
	$K/sbi.o				\
	$K/printf.o				\
	$K/interrupt.o			\
	$K/timer.o				\
	$K/heap.o				\
	$K/memory.o				\
	$K/mapping.o			\
	$K/thread.o				\
	$K/threadpool.o			\
	$K/processor.o			\
	$K/rrscheduler.o		\
	$K/syscall.o			\
	$K/elf.o				\
	$K/string.o				\
	$K/fs.o					\
	$K/queue.o				\
	$K/condition.o			\
	$K/stdin.o				\
	$K/main.o

UPROSBASE =					\
	$U/entry.o				\
	$U/malloc.o				\
	$U/io.o					\
	$U/string.o				\

UPROS =						\
	hello					\
	fib50					\
	sh

# 设置交叉编译工具链
TOOLPREFIX := riscv64-linux-gnu-
ifeq ($(shell uname),Darwin)
	TOOLPREFIX=riscv64-unknown-elf-
endif
CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump

# QEMU 虚拟机
QEMU = qemu-system-riscv64

# gcc 编译选项
CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -I.

CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

# ld 链接选项
LDFLAGS = -z max-page-size=4096

# QEMU 启动选项
QEMUOPTS = -machine virt -bios default -device loader,file=Image,addr=0x80200000 --nographic

all: Image

Image: Kernel

Kernel: User $(subst .c,.o,$(wildcard $K/*.c))
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $K/Kernel $(OBJS)
	$(OBJCOPY) $K/Kernel -O binary Image

User: mksfs $(subst .c,.o,$(wildcard $U/*.c))
	mkdir -p rootfs/bin
	for file in $(UPROS); do											\
		$(LD) $(LDFLAGS) -o rootfs/bin/$$file $(UPROSBASE) $U/$$file.o;	\
	done
	./mksfs

mksfs:
	gcc mkfs/mksfs.c -o mksfs

# compile all .c file to .o file
$K/%.o: $K/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$U/%.o: $U/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f */*.d */*.o $K/Kernel Image Image.asm mksfs fs.img
	rm -rf rootfs
	
asm: Kernel
	$(OBJDUMP) -S $K/Kernel > Image.asm

qemu: Image
	$(QEMU) $(QEMUOPTS)