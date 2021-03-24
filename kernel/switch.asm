.altmacro
# 寄存器宽度 8 字节
.set    REG_SIZE, 8
# Context 大小为 34 个寄存器大小
.set    CONTEXT_SIZE, 13*REG_SIZE

# 宏：保存寄存器到栈上
.macro SAVE reg, offset
    sd  \reg, \offset*8(sp)
.endm

.macro SAVE_N n
    SAVE  s\n, \n
.endm

# 宏：从栈中恢复寄存器
.macro LOAD reg, offset
    ld  \reg, \offset*8(sp)
.endm

.macro LOAD_N n
    LOAD  s\n, \n
.endm

    # 将程序上下文保存在栈上
    addi  sp, sp, -CONTEXT_SIZE
    sd sp, 0(a0)
    .set    n, 0
    .rept   12
        SAVE_N  %n
        .set    n, n + 1
    .endr
    sd ra, 12*REG_SIZE(sp)

    # 从目标线程栈上恢复上下文
    ld sp, 0(a1)
    .set    n, 0
    .rept   12
        LOAD_N  %n
        .set    n, n + 1
    .endr
    ld ra, 12*REG_SIZE(sp)
    addi sp, sp, CONTEXT_SIZE

    ret