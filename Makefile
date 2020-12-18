K=kernel

OBJS = 						\
	$K/sbi.o				\
	$K/printf.o				\
	$K/interrupt.o			\
	$K/timer.o				\
	$K/heap.o				\
	$K/memory.o				\
	$K/mapping.o			\
	$K/thread.o				\
	$K/main.o

# 设置交叉编译工具链
TOOLPREFIX := riscv64-linux-gnu-
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

Kernel: $(subst .c,.o,$(wildcard $K/*.c))
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $K/Kernel $(OBJS)
	$(OBJCOPY) $K/Kernel -O binary Image

# compile all .c file to .o file
$K/%.o: $K/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f */*.d */*.o $K/Kernel Image Image.asm
	
asm: Kernel
	$(OBJDUMP) -S $K/Kernel > Image.asm

qemu: Image
	$(QEMU) $(QEMUOPTS)