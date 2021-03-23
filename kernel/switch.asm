.equ XLENB, 8

    # 将程序上下文保存在栈上
    addi  sp, sp, (-XLENB*13)
    sd sp, 0(a0)
    sd ra, 0*XLENB(sp)
    sd s0, 1*XLENB(sp)
    sd s1, 2*XLENB(sp)
    sd s2, 3*XLENB(sp)
    sd s3, 4*XLENB(sp)
    sd s4, 5*XLENB(sp)
    sd s5, 6*XLENB(sp)
    sd s6, 7*XLENB(sp)
    sd s7, 8*XLENB(sp)
    sd s8, 9*XLENB(sp)
    sd s9, 10*XLENB(sp)
    sd s10, 11*XLENB(sp)
    sd s11, 12*XLENB(sp)

    # 从目标线程栈上恢复上下文
    ld sp, 0(a1)
    ld ra, 0*XLENB(sp)
    ld s0, 1*XLENB(sp)
    ld s1, 2*XLENB(sp)
    ld s2, 3*XLENB(sp)
    ld s3, 4*XLENB(sp)
    ld s4, 5*XLENB(sp)
    ld s5, 6*XLENB(sp)
    ld s6, 7*XLENB(sp)
    ld s7, 8*XLENB(sp)
    ld s8, 9*XLENB(sp)
    ld s9, 10*XLENB(sp)
    ld s10, 11*XLENB(sp)
    ld s11, 12*XLENB(sp)
    addi sp, sp, (XLENB*13)

    ret