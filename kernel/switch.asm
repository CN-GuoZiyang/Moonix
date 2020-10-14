.equ XLENB, 8

    addi  sp, sp, (-XLENB*14)
    sd sp, 0(a0)
    sd ra, 0*XLENB(sp)
    sd s0, 2*XLENB(sp)
    sd s1, 3*XLENB(sp)
    sd s2, 4*XLENB(sp)
    sd s3, 5*XLENB(sp)
    sd s4, 6*XLENB(sp)
    sd s5, 7*XLENB(sp)
    sd s6, 8*XLENB(sp)
    sd s7, 9*XLENB(sp)
    sd s8, 10*XLENB(sp)
    sd s9, 11*XLENB(sp)
    sd s10, 12*XLENB(sp)
    sd s11, 13*XLENB(sp)
    csrr  s11, satp
    sd s11, 1*XLENB(sp)

    ld sp, 0(a1)
    ld s11, 1*XLENB(sp)
    csrw satp, s11
    sfence.vma
    ld ra, 0*XLENB(sp)
    ld s0, 2*XLENB(sp)
    ld s1, 3*XLENB(sp)
    ld s2, 4*XLENB(sp)
    ld s3, 5*XLENB(sp)
    ld s4, 6*XLENB(sp)
    ld s5, 7*XLENB(sp)
    ld s6, 8*XLENB(sp)
    ld s7, 9*XLENB(sp)
    ld s8, 10*XLENB(sp)
    ld s9, 11*XLENB(sp)
    ld s10, 12*XLENB(sp)
    ld s11, 13*XLENB(sp)
    addi sp, sp, (XLENB*14)

    sd zero, 0(a1)
    ret
