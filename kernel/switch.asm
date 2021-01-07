/*
 *  kernel/switch.asm
 *  
 *  (C) 2021  Ziyang Guo
 */

.equ    REG_SIZE, 8
.altmacro
# 宏：保存寄存器到栈上
.macro SAVE reg, offset
    sd  \reg, \offset*REG_SIZE(sp)
.endm

# 将寄存器 s_n 保存到栈的 (n+2) * REGSIZE 位置
.macro SAVE_N n
    SAVE  s\n, (\n+2)
.endm

# 宏：从栈中恢复寄存器
.macro LOAD reg, offset
    ld  \reg, \offset*REG_SIZE(sp)
.endm

.macro LOAD_N n
    LOAD  s\n, (\n+2)
.endm

    # 分配栈空间，用于保存 ThreadContext
    addi    sp, sp, (-REG_SIZE*14)
    # 更新入参的当前线程栈顶地址
    sd      sp, 0(a0)
    # 依次保存各个寄存器的值
    SAVE    ra, 0
    .set    n, 0
    .rept   12
        SAVE_N  %n
        .set    n, n + 1
    .endr
    csrr    s11, satp
    SAVE    s11, 1

    # 准备恢复到目标线程，首先切换栈
    ld      sp, 0(a1)
    LOAD    s11, 1
    # 恢复页表寄存器并刷新 TLB
    csrw    satp, s11
    sfence.vma
    # 依次加载各个寄存器
    LOAD    ra, 0
    .set    n, 0
    .rept   12
        LOAD_N  %n
        .set    n, n + 1
    .endr
    # 回收栈空间
    addi    sp, sp, (REG_SIZE*14)

    sd      zero, 0(a1)
    ret
