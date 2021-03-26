# 规定：
# 若在中断之前处于 U-Mode
# 则 sscratch 保存的是内核栈地址
# 否则中断之前处于 S-Mode
# sscratch 保存的是 0

.altmacro
# 寄存器宽度 8 字节
.set    REG_SIZE, 8
# Context 大小为 34 个寄存器大小
.set    CONTEXT_SIZE, 34*REG_SIZE

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
    csrrw sp, sscratch, sp
    # 如果 sp = 0，即之前的 sscratch = 0
    # 说明是从 S-Mode 进入中断，不需要切换栈，需要再把 sscratch 的内容读到 sp
    # 此时 sp 和 sscratch 中都是内核栈地址
    # 否则就是从 U-Mode 进入中断，上一条语句将 sp 与 sscratch 
    # 此时的 sp 就是内核栈地址，sscratch 就是用户栈地址
    bnez    sp, from_user
from_kernel:
    csrr    sp, sscratch
from_user:
    # 移动栈指针，留出 Context 的空间
    addi    sp, sp, -CONTEXT_SIZE
    # 保存通用寄存器，其中 x0 固定为 0
    SAVE    x1, 1
    # 若从 S-Mode 进入中断，此时 sscratch 为内核栈地址
    # 若从 U-Mode 进入中断，此时 sscratch 为用户栈地址
    # 将 sscratch 保存到栈上，并清空
    csrrw   s0, sscratch, x0
    SAVE    s0, 2
    # 循环保存 x3 至 x31
    .set    n, 3
    .rept   29
        SAVE_N  %n
        .set    n, n + 1
    .endr

    # 保存 CSR
    csrr    s1, sstatus
    csrr    s2, sepc
    SAVE    s1, 32
    SAVE    s2, 33

    # 调用 handleInterrupt()
    # 将 scause 作为参数传入
    mv      a0, sp
    csrr    a1, scause
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
    # 按照规定
    # 如果从 U-Mode 进入中断，则需要让 sscratch 指向内核栈顶
    # 如果从 S-Mode 进入中断，不会执行这一步，sscratch 在上面已经清零，符合规定
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

    # 如果从 U-Mode 进入中断，这一步切换回用户栈
    # 如果从 S-Mode 进入中断，这一步切换(回)内核栈
    LOAD    x2, 2
    # 跳转到 sepc 中的地址
    sret
