# 🌕 Moonix

> Yet another RISC-V operating system in C

本文档将展示如何从零开始使用 ANSI C 编写出一个基于 64 位 RISC-V 架构的操作系统，你可以在该系统内核上运行用户态（User/Application mode）的终端，并输入命令执行其他程序。

本文档分别部署在 Github 和 Gitee 上，国内用户可以访问 Gitee 版。

- https://cn-guoziyang.github.io/Moonix/
- https://cn-guoziyang.gitee.io/moonix/

 当前章节教程进度：

 - [x] Chapter -1 前置知识
    - [x] -1.1 RISC-V 硬件机制
    - [x] -1.2 RISC-V 汇编
    - [x] SBI 接口简介
    - [x] GDB 调试简介
    - [x] 整体架构
- [x] Chapter 0 实验环境
- [x] Chapter 1 最小内核
    - [x] 1.1 内核入口点
    - [x] 1.2 生成内核镜像
    - [x] 1.3 使用 QEMU 运行
    - [x] 1.4 封装 SBI 接口
- [x] Chapter 2 开启中断
    - [x] 2.1 中断概述
    - [x] 2.2 触发断点
    - [x] 2.3 程序运行上下文
    - [x] 2.4 开启时钟中断
- [x] Chapter 3 内存管理
    - [x] 3.1 Buddy System
    - [x] 3.2 动态内存分配
    - [x] 3.3 按页分配框架
    - [x] 3.4 基于线段树的页帧分配
- [x] Chapter 4 虚拟内存
    - [x] 4.1 Sv39
    - [x] 4.2 内核初始映射
    - [x] 4.3 实现页表
    - [x] 4.4 内核重映射
- [x] Chapter 5 内核线程
    - [x] 5.1 线程定义
    - [x] 5.2 线程切换
    - [x] 5.3 构造线程结构
    - [x] 5.4 从启动线程到新线程
- [x] Chapter 6 线程调度
    - [x] 6.1 线程管理
    - [x] 6.2 调度线程
    - [x] 6.3 Round-Robin 算法
    - [x] 6.4 调度测试
- [x] Chapter 7 用户线程
    - [x] 7.1 创建用户程序
    - [x] 7.2 实现系统调用
    - [x] 7.3 进程内存空间
    - [x] 7.4 创建用户进程
- [x] Chapter 8 文件系统
    - [x] 8.1 SimpleFS
    - [x] 8.2 打包镜像
    - [x] 8.3 内核文件驱动
    - [x] 8.4 文件系统测试
- [x] Chapter 9 实现终端
    - [x] 9.1 键盘中断
    - [x] 9.2 条件变量与输入缓冲
    - [x] 9.3 echo 程序
    - [x] 9.4 终端！
- [ ] 鸣谢