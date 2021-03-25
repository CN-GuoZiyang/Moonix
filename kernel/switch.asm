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
    # 保存 sp 寄存器（旧线程上下文）
    # a0 寄存器即为函数参数中的 from，即修改 from 的值
    sd sp, 0(a0)
    # 保存 12 个 callee-saved 寄存器
    .set    n, 0
    .rept   12
        SAVE_N  %n
        .set    n, n + 1
    .endr
    # 保存返回地址寄存器到栈上
    SAVE ra, 12

    # 从目标线程栈上恢复上下文，首先需要将栈顶位置加载到 sp 中
    # a1 寄存器即为函数参数中的 to
    ld sp, 0(a1)
    # 恢复 callee-saved 寄存器
    .set    n, 0
    .rept   12
        LOAD_N  %n
        .set    n, n + 1
    .endr
    # 恢复 ra 寄存器
    LOAD ra, 12
    addi sp, sp, CONTEXT_SIZE

    ret
