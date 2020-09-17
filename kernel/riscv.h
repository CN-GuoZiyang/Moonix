#ifndef _RISCV_H
#define _RISCV_H

// write the Supervisor scratch register for early trap handler
static inline void
w_sscratch(uint64 x)
{
    asm volatile("csrw sscratch, %0" : : "r" (x));
}

// Supervisor Trap-Vector Base Address
// low two bits are mode.
static inline void 
w_stvec(uint64 x)
{
    asm volatile("csrw stvec, %0" : : "r" (x));
}

// Supervisor Trap Cause
static inline uint64
r_scause()
{
    uint64 x;
    asm volatile("csrr %0, scause" : "=r" (x) );
    return x;
}

// machine exception program counter, holds the
// instruction address to which a return from
// exception will go.
static inline void 
w_sepc(uint64 x)
{
    asm volatile("csrw sepc, %0" : : "r" (x));
}

static inline uint64
r_sepc()
{
    uint64 x;
    asm volatile("csrr %0, sepc" : "=r" (x) );
    return x;
}

#endif