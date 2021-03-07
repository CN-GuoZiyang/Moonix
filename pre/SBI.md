# SBI 规范

本章将对 SBI（Supervisor Binary Interface，监管者模式二进制接口）规范进行简要介绍。图文主要来自 RISC-V 官方的 (SBI 文档)[https://github.com/riscv/riscv-sbi-doc]。

## 介绍

RISC-V SBI 规范描述了 RISC-V 监管者模式二进制接口（SBI）的实现规范。SBI 通过定义平台（或虚拟机管理器）特定功能的抽象，来允许监管者模式（S-Mode 或 VS-Mode）软件可以在所有 RISC-V 平台进行移植。SBI 的设计遵循了RISC-V 的设计哲学，拥有一个小的核心和一组可选的模块扩展。

向监管者模式软件提供 SBI 接口的高特权级软件，被称为一个 **SBI 实现**，或者**监管者执行环境**（Supervisor Execution Environment，SEE）。SBI 实现可以是一个在机器模式（M-Mode）下运行的平台运行时固件（如图 1），也可以是在虚拟机管理模式（HS-Mode）下运行的虚拟机管理程序（如图 2）。

![无 H 扩展的 RISC-V 系统](https://raw.githubusercontent.com/riscv/riscv-sbi-doc/master/riscv-sbi-intro1.png)
图1. 不带 H 扩展的 RISC-V 系统

![有 H 扩展的 RISC-V 系统](https://raw.githubusercontent.com/riscv/riscv-sbi-doc/master/riscv-sbi-intro2.png)
图2. 带 H 扩展的 RISC-V 系统

## 二进制编码

所有的 SBI 函数都使用相同的二进制编码，这有助于搭配不同的 SBI 扩展使用。这种二进制编码与 RISC-V Linux 系统调用 ABI 相同，后者本身就基于 RISC-V ELF psABI 中定义的调用约定。换句话说，SBI 调用与标准 RISC-V 函数调用完全相同，除了：

- 使用 `ECALL` 指令作为控制流程转移指令，而不是 `CALL` 指令。
- `a7` 寄存器编码了 SBI 扩展 ID (EID)，它与 Linux 系统调用 ABI 中系统调用号的编码方式相匹配。

许多 SBI 扩展还选择在 `a6` 寄存器中编码附加的 SBI 函数 ID (FID)，这种方案类似于许多 UNIX 操作系统上的 `ioctl()` 系统调用。这允许 SBI 扩展在单个扩展空间中编码多个函数。

为了兼容，SBI 扩展 id（EID）和 SBI 函数 id（fid）被编码为带符号的 32 位整数。当传入寄存器时，它们遵循标准的 RISC-V 调用约定规则。

SBI 函数必须返回一对值，存储在 `a0` 和 `a1` 寄存器中，其中 `a0` 是一个错误代码。这类似于 C 结构体：

```c
struct sbiret {
    long error;
    long value;
};
```

SBI 错误代码编码如下：

|错误类型|值|
|:--:|:--:|
|SBI_SUCCESS|0|
|SBI_ERR_FAILED|-1|
|SBI_ERR_NOT_SUPPORTED|-2|
|SBI_ERR_INVALID_PARAM|-3|
|SBI_ERR_DENIED|-4|
|SBI_ERR_INVALID_ADDRESS|-5|
|SBI_ERR_ALREADY_AVAILABLE|-6|

每个 SBI 函数都应该选择 unsigned long 作为数据类型。它保持规范简单，易于适应所有 RISC-V ISA 类型（即 RV32、RV64 和 RV128）。如果数据被定义为32位宽，高权限软件必须保证它只使用这 32 位数据。

## 基本扩展（EID #0x10）

基本扩展部分被设计得尽可能小。因此，它只包含探测哪些 SBI 扩展可用以及查询 SBI 版本的功能。所有 SBI 实现都必须支持基本扩展中的所有函数，因此没有定义错误返回。

### 获取 SBI 规范版本（FID #0）

```c
struct sbiret sbi_get_spec_version(void);
```

返回当前的 SBI 规范版本。这个函数必须总是调用成功。SBI 规范的副编号在低 24 位中编码，主编号在之后的 7 位中编码。第 31 位必须为 0，为将来扩展预留。

### 获取 SBI 实现 ID（FID #1）

```c
struct sbiret sbi_get_impl_id(void);
```

返回当前的 SBI 实现 ID，这个 ID 对于每个 SBI 实现都是不同的。这个实现 ID 允许软件探测 SBI 实现的特殊性。

### 获取 SBI 实现版本（FID #2）

```c
struct sbiret sbi_get_impl_version(void);
```

返回当前的 SBI 实现版本。这个版本号的编码是基于特定的 SBI 编码的。

### 检测 SBI 扩展（FID #3）

```c
struct sbiret sbi_probe_extension(long extension_id);
```

如果给定的 SBI 扩展 ID（EID）不可用，则返回 0；如果可用，则返回基于扩展的非零值。

### 获取机器供应商 ID（FID #4）

```c
struct sbiret sbi_get_mvendorid(void);
```

返回一个对于 `mvendorid` CSR 是合法的值，0 始终是这个 CSR 的合法值。

### 获取机器结构 ID（FID #5）

```c
struct sbiret sbi_get_marchid(void);
```

返回一个对于 `marchid` CSR 是合法的值，0 始终是这个 CSR 的合法值。

### 获取机器实现 ID（FID #6）

```c
struct sbiret sbi_get_mimpid(void);
```

返回一个对于 `mimpid` CSR 是合法的值，0 始终是这个 CSR 的合法值。

### SBI 实现 ID

|实现 ID|名称|
|:--:|:--:|
|0|BBL|
|1|OpenSBI|
|2|Xvisor|
|3|KVM|
|4|RustSBI|
|5|Diosix|

## 传统扩展（EID #0x00 - #0x0F）

传统 SBI 扩展忽略了 SBI 函数 ID 域，而被编码为多个 SBI 扩展 ID。

### 设置时钟（EID #0x00）

```c
void sbi_set_timer(uint64_t stime_value)
```

为 stime_value 时间之后的下一个事件设置时钟。这个函数也清除挂起的时钟中断位。

如果一个监管者软件想要清除时钟中断，而不设置下一个时钟事件，可以设置一个时钟中断到无限远的未来（即(uint64_t)-1），或者它可以通过清除 sie 寄存器的 STIE 位来屏蔽时钟中断。

### 控制台输出（EID #0x01）

```c
void sbi_console_putchar(int ch)
```

将 ch 代表的字符输出到调试控制台。

与 `sbi_console_getchar()` 不同，如果还有待传输的字符存在，或者接收终端还没有准备好接收字节，那么这个函数调用将被阻塞。然而，如果终端不存在，那么这个字符就会被直接丢弃并返回。

### 控制台输入（EID #0x02）

```c
int sbi_console_getchar(void)
```

从调试控制台中读入一个字节。如果读入成功，则返回这个字节，如果失败返回 -1。注意，这是传统扩展中唯一一个返回非空数据的的 SBI 调用。

### 清空 IPI（EID #0x03）

```c
void sbi_clear_ipi(void)
```

清空所有正在等待的 IPI 中断。只有调用这个 SBI 调用的硬件线程的 IPI 中断会被清除。

### 发送 IPI（EID #0x04）

```c
void sbi_send_ipi(const unsigned long *hart_mask)
```

向 hart_mask 中定义的所有硬件线程发送一个处理器间中断。处理器间中断以 S-Mode 软件中断的形式出现在接收端。

### 远程 `FENCE.I`（EID #0x05）

```c
void sbi_remote_fence_i(const unsigned long *hart_mask)
```

指示远程硬件线程执行 `FENCE.I` 指令。hart_mask 参数和 `sbi_send_ipi()` 函数中描述的一样。

### 远程 `SFENCE.VMA`（EID #0x06）

```c
void sbi_remote_sfence_vma(const unsigned long *hart_mask,
                           unsigned long start,
                           unsigned long size)
```

指示远程硬件线程执行一条或多条 `SFENCE.VMA` 指令，范围是 start 和 size 指定的虚拟地址空间。

### 远程带 ASID 的 `SFENCE.VMA` 指令（EID #0x07）

```c
void sbi_remote_sfence_vma_asid(const unsigned long *hart_mask,
                                unsigned long start,
                                unsigned long size,
                                unsigned long asid)
```

指示远程硬件线程执行一条或多条 `SFENCE.VMA` 指令，范围是 start 和 size 指定的虚拟地址空间。这只包含指定的 ASID。

### 关闭系统（EID #0x08）

```c
void sbi_shutdown(void)
```

从监管者模式的角度关闭所有的硬件线程。这个 SBI 调用不会返回。

本教程只会涉及前两个扩展，更多 SBI 规范内容请参阅 [Github](https://github.com/riscv/riscv-sbi-doc/blob/master/riscv-sbi.adoc)。