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
    csrrw   sp, sscratch, sp
    bnez    sp, from_user
from_kernel:
    csrr    sp, sscratch
from_user:
    # 移动栈指针，留出 Context 的空间
    addi    sp, sp, -34*REG_SIZE
    
    # 保存通用寄存器，其中 x0 固定为 0
    SAVE    x1, 1
    # 循环保存 x3 至 x31
    .set    n, 3
    .rept   29
        SAVE_N  %n
        .set    n, n + 1
    .endr

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

    andi    s0, s1, 1 << 8
    bnez    s0, to_kernel
to_user:
    addi    s0, sp, 34 * REG_SIZE
    csrw    sscratch, s0
to_kernel:
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

    # 恢复 sp（这里最后恢复是为了上面可以正常使用 LOAD 宏）
    LOAD    x2, 2
    sret