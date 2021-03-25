K=kernel

# 后续添加的源文件需要在这里添加，否则不会参与连接
OBJS =						\
	$K/entry.o				\
	$K/interrupt.o			\
	$K/thread.o				\
	$K/main.o

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
# 开启warning、将警告当成错误处理、O1优化、保留函数调用栈指针、产生GDB所需的调试信息
CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb
# 在编译过程中生成依赖文件
CFLAGS += -MD
# 设置代码模型为 medany，要求程序和相关符号都被定义在 2 GB 的地址空间中
CFLAGS += -mcmodel=medany
# 设置环境为Freestanding（不一定以main为入口）、未初始化全局变量放在bss段、链接时不使用标准库、减少获取符号地址所需的指令数
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -I.
# 关闭 gcc 的栈溢出保护机制
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

# ld 链接选项
LDFLAGS = -z max-page-size=4096

# QEMU 启动选项
# 设置虚拟机类型为 virt、bios 使用默认 BIOS（OpenSBI）
# 将镜像复制到物理地址 0x80200000 处
# 以无界面模式启动（不支持 VGA）
QEMUOPTS = -machine virt -bios default -device loader,file=Image,addr=0x80200000 --nographic

all: Image

Image: Kernel

# 将所有可执行文件链接成内核，并生成内核镜像
Kernel: $(subst .c,.o,$(wildcard $K/*.c)) $(subst .S,.o,$(wildcard $K/*.S))
	$(LD) $(LDFLAGS) -T $K/kernel.ld -o $K/Kernel $(OBJS)
	$(OBJCOPY) $K/Kernel -O binary Image

# 编译所有的 .c 文件
$K/%.o: $K/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 编译所有的 .S 文件
$K/%.o: $K/%.S
	$(CC) $(CFLAGS) -c $< -o $@

# 清空编译内核的临时文件
clean:
	rm -f */*.d */*.o $K/Kernel Image Image.asm

# 生成内核的反汇编	
asm: Kernel
	$(OBJDUMP) -S $K/Kernel > Image.asm

# 启动 QEMU 加载内核执行
qemu: Image
	$(QEMU) $(QEMUOPTS)

GDBPORT = $(shell expr `id -u` % 5000 + 25000)
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)

# 开启 qemu 的 gdb 后端
qemu-gdb: Image asm
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB)