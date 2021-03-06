# 第〇章 实验环境

----

# 概要

本章主要介绍如何在常用的 Linux 发行版下安装实验所需的工具，主要分为两个部分：交叉编译工具链和 QEMU 。本人使用 Manjaro 进行实验，这是一款基于 Arch Linux 的用户友好的发行版。安装实验环境部分已在 Manjaro 下完成测试。

# 安装交叉编译工具链

通常，开发一个操作系统的环境并不是目标环境。例如，本次开发的 RISC-V 架构的操作系统，就并不是在 RISC-V 架构的计算机上开发的。大多数同学的电脑都是 x86 架构的，可能有个别同学的是 Arm 架构。
> [!TIP|label:Note|]
> 交叉编译器（Cross compiler）是指一个在某个系统平台下可以产生另一个系统平台的可执行文件的编译器。交叉编译器在目标系统平台（开发出来的应用程序序所运行的平台）难以或不容易编译时非常有用。  ——Wikipedia

借助交叉编译器，我们在自己的 x86 或 Arm 的 PC 上就可以编译出 RISC-V 平台的可执行文件，而不需要真正地拥有一台 RISC-V 架构的机器。

这一套工具链主要包括gcc、ld、objdump 和 gdb 等。

## 在基于 Arch Linux 的发行版下安装

基于 Arch Linux 的发行版包括 Arch Linux 本体与 Manjaro 等其他衍生发行版，这类发行版使用 pacman 作为包管理器。

使用如下命令安装交叉编译工具链：

```bash
$ sudo pacman -S riscv64-linux-gnu-binutils riscv64-linux-gnu-gcc riscv64-linux-gnu-gdb
```

## 在基于 Debian 的发行版下安装

基于 Debian 的发行版包括 Debian 本体与 Ubuntu、Linux Mint 与 Deepin 等其他衍生发行版，这类发行版使用 Apt 作为包管理器。

使用如下命令安装交叉编译工具链：

```bash
$ sudo apt install git build-essential gdb-multiarch gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu
```

## 在 MacOS 下安装

在 MacOS 下需要借助 Homebrew 这个包管理器：

```bash
$ /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

随后使用 Homebrew 安装工具链：

```bash
$ brew tap riscv/riscv
$ brew install riscv-tools
```

注意：MacOS 下的工具链前缀为 **riscv64-unknown-elf-**

## 验证工具链

为了验证工具链安装成功，可以执行如下指令，输出类似内容即为安装成功：

```bash
$ riscv64-linux-gnu-gcc --version
riscv64-linux-gnu-gcc (GCC) 10.2.0
Copyright (C) 2020 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

MacOS 下为：

```bash
$ riscv64-unknown-elf-gcc --version
riscv64-unknown-elf-gcc (GCC) 10.2.0
Copyright (C) 2020 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

# 安装 QEMU

由于实际上我们**并没有** RISC-V 架构的机器，即使真的编写出了什么操作系统或者程序，也没有硬件平台来运行。好在，随着 RISC-V 架构的流行，很多虚拟机开始支持对 RISC-V 架构硬件的模拟，如 Bochs 和 QEMU 等。这里我们选用 QEMU。

这里不推荐使用包管理器中的 QEMU。新版本的 QEMU 已经默认集成了 OpenSBI 作为 BIOS，而较早版本的 QEMU 需要自行编译 OpenSBI。关于 OpenSBI 的内容会在后续章节中介绍。

要安装新版本的 QEMU，我们只能从源码编译。

## 获取 QEMU 源码

首先从 Github 上获取 QEMU 的源码：

```bash
$ git clone https://github.com/qemu/qemu
```

如果由于某些原因速度较慢，可以使用 Gitee 提供的镜像：

```bash
$ git clone https://gitee.com/mirrors/qemu.git
```

Clone 完成后进入源码文件夹。我们需要使用 QEMU v5.0.0 版本，更新的版本可能会有一些意想不到的特性（最新版本的 qemu 似乎无法运行）。

```bash
$ git checkout fdd76fecdde1ad444ff4deb7f1c4f7e4a1ef97d6
```

## 编译 QEMU

配置编译选项并安装到系统：

```bash
$ ./configure --target-list=riscv64-softmmu
$ make
$ sudo make install
```

在配置过程中可能速度较慢，配置脚本会自动从官方仓库中 clone 一些子模块，网络不好的话可能会导致失败。建议使用科学上网，或者由可以科学上网的小伙伴编译完成后拷贝过去。

> [!WARNING]
> 如果你使用的是较新版本的 gcc（一些滚动发行版可能会直接使用最新的 gcc，如 Arch 系发行版），在 make 过程中可能会出现 `ld: Error: unable to disambiguate: -no-pie (did you mean --no-pie ?)`的错误。
>
> 解决方案：将本项目 master 分支下 patch 文件夹中的 所有 .img 文件复制到 qemu 文件夹下的 pc-bios/optionrom/ ，并再次 make 即可

## 验证 QEMU 安装

安装完成后，可以使用如下命令启动 QEMU，输出类似字段即安装成功。

```bash
$ qemu-system-riscv64 \
-nographic \
-machine virt \
-bios default

OpenSBI v0.6
   ____                    _____ ____ _____
  / __ \                  / ____|  _ \_   _|
 | |  | |_ __   ___ _ __ | (___ | |_) || |
 | |  | | '_ \ / _ \ '_ \ \___ \|  _ < | |
 | |__| | |_) |  __/ | | |____) | |_) || |_
  \____/| .__/ \___|_| |_|_____/|____/_____|
        | |
        |_|

Platform Name          : QEMU Virt Machine
Platform HART Features : RV64ACDFIMSU
Platform Max HARTs     : 8
Current Hart           : 0
Firmware Base          : 0x80000000
Firmware Size          : 120 KB
Runtime SBI Version    : 0.2

MIDELEG : 0x0000000000000222
MEDELEG : 0x000000000000b109
PMP0    : 0x0000000080000000-0x000000008001ffff (A)
PMP1    : 0x0000000000000000-0xffffffffffffffff (A,R,W,X)
```

在此界面按下 `Ctrl + A` 再按下 `X` 即可退出。

注意提示的 OpenSBI 版本是 v0.6，如版本不对应可能是没有切换到对应分支，请运行 `git checkout` 命令并重新编译安装。