/*
 *  kernel/entry.asm
 *  
 *  (C) 2021  Ziyang Guo
 */

 /*
  * entry.asm 是程序的入口，被放置在物理内存 0x80200000 处
  * 主要用于设置内核启动栈并跳转到 main 函数
  */

    .section .text.entry
    .globl _start
_start:
    # 计算 bootpagetable 的物理页号（satp 中保存的页表基地址是物理页号）
    lui t0, %hi(bootpagetable)
    li t1, 0xffffffff00000000
    sub t0, t0, t1
    srli t0, t0, 12
    # 设置使用 SV39（8 << 60）
    li t1, (8 << 60)
    or t0, t0, t1
    # 写入 satp 并刷新 TLB
    csrw satp, t0
    sfence.vma

    # 加载栈地址，I 型指令只支持最多 32 位立即数，操作地址时需要分两次装载
    lui sp, %hi(bootstacktop)
    addi sp, sp, %lo(bootstacktop)

    # 跳转到 main
    lui t0, %hi(main)
    addi t0, t0, %lo(main)
    jr t0

    .section .stack
    .align 12

    # 以下 4096 × 16 字节的空间作为 OS 的启动栈
    .global bootstack
bootstack:
    .space 4096 * 16
    .global bootstacktop
bootstacktop:

    # 初始内核映射所用的页表
    .section .data
    .align 12
bootpagetable:
    .quad 0
    .quad 0
    # 第 2 项：0x80000000 -> 0x80000000，0xcf 表示 VRWXAD 均为 1，该页表项用于支持跳转到 main 之前刷新 TLB 之后的代码工作
    .quad (0x80000 << 10) | 0xcf
    .zero 507 * 8
    # 第 510 项：0xffffffff80000000 -> 0x80000000，0xcf 表示 VRWXAD 均为 1
    .quad (0x80000 << 10) | 0xcf
    .quad 0
