.equ XLENB, 8
.macro Load reg, mem
    ld \reg, \mem
.endm
.macro Store reg, mem
    sd \reg, \mem
.endm
    addi sp, sp, -(XLENB*14)
    sd sp, 0(a0)
    Store ra, 0*XLENB(sp)
    Store s0, 2*XLENB(sp)
    Store s1, 3*XLENB(sp)
    Store s2, 4*XLENB(sp)
	Store s3, 5*XLENB(sp)
	Store s4, 6*XLENB(sp)
	Store s5, 7*XLENB(sp)
	Store s6, 8*XLENB(sp)
	Store s7, 9*XLENB(sp)
	Store s8, 10*XLENB(sp)
	Store s9, 11*XLENB(sp)
    Store s10, 12*XLENB(sp)
    Store s11, 13*XLENB(sp)
    csrr s11, satp
    Store s11, 1*XLENB(sp)

    ld sp, 0(a1)
    Load s11, 1*XLENB(sp)
    csrw satp, s11
    Load ra, 0*XLENB(sp)
    Load s0, 2*XLENB(sp)
    Load s1, 3*XLENB(sp)
    Load s2, 4*XLENB(sp)
	Load s3, 5*XLENB(sp)
	Load s4, 6*XLENB(sp)
	Load s5, 7*XLENB(sp)
	Load s6, 8*XLENB(sp)
	Load s7, 9*XLENB(sp)
	Load s8, 10*XLENB(sp)
	Load s9, 11*XLENB(sp)
    Load s10, 12*XLENB(sp)
    Load s11, 13*XLENB(sp)
    mv a0, s0
    csrw sstatus, s1
    addi sp, sp, 14*XLENB

    Store zero, 0(a1)
    ret