/*
 *  kernel/interrupt.asm
 *  
 *  (C) 2021  Ziyang Guo
 */

/* 
 * interrupt.asm 是中断处理程序的入口
 * 由于 S-Mode 中断被设置为 Direct 模式，所有的中断发生时都会执行这个程序
 * 主要过程是在中断发生时将当前程序的上下文保存在内核栈上，并在中断处理完成后恢复
 */

.altmacro
# 寄存器宽度 8 字节
.set    REG_SIZE, 8
# Context 大小为 34 字节
.set    CONTEXT_SIZE, 34

# 宏：保存寄存器到栈上
.macro SAVE reg, offset
    sd  \reg, \offset*8(sp)
.endm

.macro SAVE_N n
    SAVE  x\n, \n
.endm

# 宏：从栈中恢复寄存器
.macro LOAD reg, offset
    ld  \reg, \offset*8(sp)
.endm

.macro LOAD_N n
    LOAD  x\n, \n
.endm


    .section .text
    .globl __interrupt
    # 中断处理函数需要 4 字节对齐
    .balign 4
# 全局中断处理，保存 Context 并跳转到 handleInterrupt() 处
__interrupt:
    # 交换 sscratch 和 sp
    csrrw   sp, sscratch, sp

    # 如果 sp = 0，即之前的 sscratch = 0
    # 说明是从 S-Mode 进入中断，不需要切换栈
    # 否则需要将 sp 从用户栈切换到内核栈
    # 此时的 sp 就是原来的 sscratch，就是内核栈顶地址
    # 否则还需要执行 csrr 
    bnez    sp, from_user
from_kernel:
    csrr    sp, sscratch
from_user:
    # 移动栈指针，留出 Context 的空间
    addi    sp, sp, -34*REG_SIZE
    
    # 保存通用寄存器，其中 x0 固定为 0
    SAVE    x1, 1
    # 循环保存 x3 至 x31 寄存器到栈上
    .set    n, 3
    .rept   29
        SAVE_N  %n
        .set    n, n + 1
    .endr

    # 若从 S-Mode 进入中断，此时 sscratch 为内核栈地址
    # 若从 U-Mode 进入中断，此时 sscratch 为用户栈地址
    # 将 sscratch 的值保存在 s0 中，并将 sscratch 清零
    csrrw   s0, sscratch, x0
    # 保存 CSR
    csrr    s1, sstatus
    csrr    s2, sepc

    # 将 sp、sstatus 和 sepc 保存到栈上 
    SAVE    s0, 2
    SAVE    s1, 32
    SAVE    s2, 33

    # 调用 handleInterrupt()
    # 将 Context 的地址(栈顶)和 scause、stval 作为参数传入
    mv      a0, sp
    csrr    a1, scause
    csrr    a2, stval
    jal     handleInterrupt



    .globl __restore
# 从 handleInterrupt 返回
# 从 Context 中恢复所有寄存器，并跳转至 Context 中 sepc 的位置
__restore:
    # 恢复 CSR
    LOAD    s1, 32
    LOAD    s2, 33

    # 如果从 S-Mode 进入中断， sstatus 的 SPP 位为 1
    # 如果从 U-Mode 进入中断， sstatus 的 SPP 位为 0
    andi    s0, s1, 1 << 8
    bnez    s0, to_kernel
to_user:
    # 释放内核栈空间
    addi    s0, sp, 34 * REG_SIZE
    # 如果从 U-Mode 进入中断
    # 则此时 sscratch 指向用户栈顶
    # 令其指向内核栈顶地址
    csrw    sscratch, s0
to_kernel:
    # 恢复 sstatus 和 sepc
    csrw    sstatus, s1
    csrw    sepc, s2

    # 恢复通用寄存器
    LOAD    x1, 1
    # 恢复 x3 至 x31
    .set    n, 3
    .rept   29
        LOAD_N  %n
        .set    n, n + 1
    .endr

    # 最后恢复 sp（这里最后恢复是为了上面可以正常使用 LOAD 宏）
    LOAD    x2, 2
    sret
