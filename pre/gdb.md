# GDB 调试指南

GDB，即 GNU Project Debugger，允许你在程序运行时检查内存和寄存器中的内容。只需要通过以下的方式，使用 gdb 命令来：

 - 启动程序
 - 在特定的条件下停止或暂停程序执行
 - 检查寄存器或内存空间的内容
 - 修改寄存器或内存空间的内容

本章简要介绍如何使用 gdb 来调试我们的操作系统。

## 调试之前

由于我们的操作系统是运行在虚拟机中的，所以调试的方式与调试普通程序有略微不同。我们需要借助远程调试的方式，将虚拟机当作远程机器，用本地 gdb 进行连接。

qemu 开启调试模式很简单，我已经将它写入 Makefile 中了，如下：

```makefile
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)

qemu-gdb: Image
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB)
```

只需要 `make qemu-gdb` 即可开启。开启后如下：

```bash
$ make qemu-gdb
...
riscv64-linux-gnu-objcopy kernel/Kernel -O binary Image
qemu-system-riscv64 -machine virt -bios default -device loader,file=Image,addr=0x80200000 --nographic -S -gdb tcp::26000
```

qemu会暂停启动，等待 gdb 连接。

这时可以开启另一个终端，启动一个 gdb 来连接。

arch 系发行版下开启 gdb 调试的命令为：

```bash
$ riscv64-linux-gnu-gdb kernel/Kernel
```

其他发行版在进入 gdb 后需要手动设置指令集架构为 rv64，如下：

```bash
$ gdb-multiarch kernel/Kernel
(gdb) set architecture riscv:rv64
```

注意这里我们设置的目标文件为 kernel 文件夹下的 Kernel 文件，而不是 Image 文件。这是因为在 Makefile 中，生成 Kernel 时执行了命令 `$(OBJCOPY) $K/Kernel -O binary Image`，直接生成了内存映像，同时还删除了调试信息等以减小镜像体积。所以我们在调试时还是应该使用 kernel/Kernel 文件的。

进入 gdb 调试后，需要手动连接本机的 qemu，命令如下：

```bash
(gdb) target remote 127.0.0.1:26000
```

26000 是调试端口号，在 `make qemu-gdb` 日志的最下方可以找到该端口号。

在 gdb 中输入 `quit` 命令即可退出 gdb。

下面我们来介绍一些基础用法。更全的命令可以在 [gdb 官方文档](http://sourceware.org/gdb/current/onlinedocs/gdb/) 中查找。

## 断点相关命令

既然是调试，就绕不开断点。被调试的程序会自动运行到断点处暂停，等待下一步操作。

### break 命令

用于在某一特定的行或者地址处创建一个断点。

格式：

```bash
# b 或 break 都可以使用
break [函数名]
break [文件名]:[行号]
break *[地址]
```

程序会运行到满足断点条件的位置暂停，注意这时断点不会被移除，这意味着下一次运行到同样满足条件的位置程序还会暂停，除非手动移除断点。

例如：

```bash
(gdb) target remote 127.0.0.1:26000
Remote debugging using 127.0.0.1:26000
0x0000000000001000 in ?? ()
(gdb) b main
Breakpoint 1 at 0xffffffff80202bc0: file kernel/main.c, line 23.
(gdb) c
Continuing.

Breakpoint 1, main () at kernel/main.c:23
23          printf("Initializing Moonix...\n");
(gdb) b *0xffffffff80202bd0
Breakpoint 2 at 0xffffffff80202bd0: file kernel/main.c, line 24.
(gdb) c
Continuing.

Breakpoint 2, main () at kernel/main.c:24
24          extern void initMemory();       initMemory();

# 在 main 函数和地址 0xffffffff80202bd0 处设置了断点
```

### info breakpoints 命令

格式：

```bash
info breakpoints
info breakpoints [序号]
```

第一条命令会输出所有断点的详细信息，包括地址、源文件位置以及断点触发次数。第二条命令只会输出对应序号的断点的信息。

例如：

```bash
(gdb) info breakpoints
Num     Type           Disp Enb Address            What
1       breakpoint     keep y   0xffffffff80202bc0 in main at kernel/main.c:23
breakpoint already hit 1 time
2       breakpoint     keep y   0xffffffff80202bd0 in main at kernel/main.c:24
breakpoint already hit 1 time
(gdb) info breakpoints 2
Num     Type           Disp Enb Address            What
2       breakpoint     keep y   0xffffffff80202bd0 in main at kernel/main.c:24
breakpoint already hit 1 time
```

### delete 命令

格式：

```bash
delete
delete [序号]
```

delete 命令不带参数时删除所有的断点，否则删除特定序号的断点。

例如：

```bash
(gdb) info breakpoints
Num     Type           Disp Enb Address            What
1       breakpoint     keep y   0xffffffff80202bc0 in main at kernel/main.c:23
breakpoint already hit 1 time
2       breakpoint     keep y   0xffffffff80202bd0 in main at kernel/main.c:24
breakpoint already hit 1 time
(gdb) delete
Delete all breakpoints? (y or n) y
(gdb) info breakpoints
No breakpoints or watchpoints.
```

## 程序执行控制

### Continue 命令

继续运行程序直到遇到断点或错误。

格式：

```bash
continue / c
```

举例：

```bash
(gdb) b main
Breakpoint 1 at 0xffffffff80202bc0: file kernel/main.c, line 23.
(gdb) c
Continuing.

Breakpoint 1, main () at kernel/main.c:23
23          printf("Initializing Moonix...\n");
```

### step 指令

从一个函数跳进另一个函数。即持续运行直到当前函数运行结束或当前函数调用别的函数。

格式

```bash
step / s
```

举例：

```bash
(gdb) where
#0  main () at kernel/main.c:24
(gdb) s
initMemory () at kernel/memory.c:69
69          w_sstatus(r_sstatus() | SSTATUS_SUM);
(gdb) s
r_sstatus () at kernel/riscv.h:44
44          asm volatile("csrr %0, sstatus" : "=r" (x) );
(gdb) s
initMemory () at kernel/riscv.h:51
51          asm volatile("csrw sstatus, %0" : : "r" (x));
```

### stepi 命令

执行一条机器指令并暂停

格式：

```bash
stepi / si
```

这个命令会执行一条机器指令。当指令包含一个函数调用时，命令就会进入被调用的函数。对于多线程应用，stepi 命令会让当前线程执行一条指令，同时挂起其他线程。

举例：

```bash
(gdb) si
initMemory () at kernel/memory.c:70
70          initFrameAllocator(
(gdb) si
0xffffffff80200c10      70          initFrameAllocator(
(gdb) si
0xffffffff80200c14      70          initFrameAllocator(
(gdb) si
0xffffffff80200c16      70          initFrameAllocator(
```

注意这里 gdb 不会显示出机器指令，但是可以看到指令地址在增加。

## 输出信息相关指令

### display 命令

用指定格式输出制定内存地址处的内容。

格式

```bash
x/[格式控制] [地址表达式]
x [地址表达式]
x/[长度][格式控制] [地址表达式]
```

格式控制字符：

 - o：八进制
 - x：十六进制
 - u：无符号十进制
 - t：二进制
 - f：浮点类型
 - a：地址类型
 - c：字符类型
 - s：字符串类型

举例：

```bash
(gdb) x/wx 0x80000000
0x80000000:     0x00050433
(gdb) x 0x80000000
0x80000000:     0x00050433
(gdb) x/c 0x80000000
0x80000000:     51 '3'
(gdb) x/wx 0x80000000
0x80000000:     0x00050433
(gdb) x/2hx 0x80000000
0x80000000:     0x0433  0x0005
(gdb) x/gx 0x80000000
0x80000000:     0x000584b300050433
(gdb) x/s 0x80000000
0x80000000:     "3\004\005"
```

### print 命令

输出给定表达式的值

格式

```bash
Print [表达式]
```

表达式可以是变量、内存地址、寄存器甚至是伪寄存器（如 $pc）

举例：

```bash
(gdb) print i
No symbol "i" in current context.
(gdb) print $pc
$1 = (void (*)()) 0xffffffff80200c1a <initMemory+36>
(gdb) print $ra
$5 = (void (*)()) 0xffffffff80202bd8 <main+32>
(gdb) print *0x80000000
$6 = 328755
```

### info address 命令

输出给定符号（变量或函数）的地址

格式

```bash
info address [符号名]
```

注意我们可以在程序还没有运行的情况下就可以运行这个命令。

举例

```bash
(gdb) info address main
Symbol "main" is a function at address 0xffffffff80202bb8.
(gdb) info address printf
Symbol "printf" is a function at address 0xffffffff8020016e.
```

### info registers 命令

输出所有寄存器的内容

格式

```bash
info reg / i r / info registers
info all-registers / i all-registers
info registers [寄存器名称]
```

第一条会输出除了控制寄存器（CSR）和浮点寄存器以外的所有寄存器；第二条会输出所有寄存器；第三条会输出指定的寄存器。

举例：

```bash
(gdb) i r
ra             0xffffffff80202bd8       0xffffffff80202bd8 <main+32>
sp             0xffffffff80c2cfe0       0xffffffff80c2cfe0
gp             0x0      0x0
tp             0x8001de00       0x8001de00
t0             0xffffffff80202bb8       -2145375304
t1             0x8000000000000000       -9223372036854775808
t2             0x82200000       2183135232
fp             0xffffffff80c2cff0       0xffffffff80c2cff0
s1             0x8000000000006800       -9223372036854749184
a0             0x80c2d000       2160250880
a1             0x0      0
a2             0x0      0
a3             0x1      1
a4             0x40000  262144
a5             0x100000000      4294967296
a6             0x80200000       2149580800
a7             0x1      1
s2             0x1      1
s3             0x8000000000006800       -9223372036854749184
s4             0x80200000       2149580800
s5             0x82200000       2183135232
s6             0x0      0
s7             0x0      0
s8             0x2000   8192
s9             0x0      0
s10            0x0      0
s11            0x0      0
t3             0x80200000       2149580800
t4             0x0      0
t5             0x0      0
t6             0x0      0
pc             0xffffffff80200c1a       0xffffffff80200c1a <initMemory+36>
dscratch       Could not fetch register "dscratch"; remote failure reply 'E14'
mucounteren    Could not fetch register "mucounteren"; remote failure reply 'E14'
(gdb) i r a0
a0             0x80c2d000       2160250880
(gdb) i r mie
mie            0x8      8
```

## GUI 客户端 —— cgdb

### 简介

由于 cgdb 最新的 release 有 bug，所以我用最新的源码编译了一份，保存在本仓库 master 分支下的 patch/cgdb 文件夹下。下载下来后保存到 /bin 或者 /usr/bin 目录即可在终端使用 cgdb 命令打开。

cgdb 是 GDB 的一个轻量级前端。它提供了一个分屏窗口，分别显示 gdb 命令界面（和默认 gdb 一样）和程序源码。如下：

<center>
<img src="https://cn-guoziyang.gitee.io/moonix/assets/img/cgdb.png" alt="cgdb" width=75%>
</center>

左侧的窗口被称为代码窗口，右侧为 gdb 窗口。

### 基本使用

打开 cgdb 时，默认两个窗口是上下分隔的，可以通过 `ctrl+w`切换成左右分隔模式。直接使用 cgdb 启动时，会默认使用系统中已经安装好的 gdb。由于默认的 gdb 只能调试 x86 程序，所以我们需要在启动时附带参数来改变默认的 gdb。arch 系发行版如下：

```bash
$ cgdb -d riscv64-linux-gnu-gdb kernel/Kernel
```

对于其他发行版：

```bash
$ cgdb -d gdb-multiarch kernel/Kernel
```

当然其他发行版进入后还需要手动设置架构，同使用 gdb 一致。

按 `esc` 键可以将焦点从 gdb 窗口转移到代码窗口，在代码窗口可以上下翻看源码，空格键可以在焦点行设置一个断点。

按 `i` 键可以将焦点从代码窗口转移到 gdb 窗口，在 gdb 窗口的操作与普通 gdb 完全一致。

更具体的 cgdb 使用可以查看这本[CGDB中文手册](https://leeyiw.gitbooks.io/cgdb-manual-in-chinese)。