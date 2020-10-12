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

#define SSTATUS_SPP (1L << 8)  // Previous mode, 1=Supervisor, 0=User
#define SSTATUS_SPIE (1L << 5) // Supervisor Previous Interrupt Enable
#define SSTATUS_UPIE (1L << 4) // User Previous Interrupt Enable
#define SSTATUS_SIE (1L << 1)  // Supervisor Interrupt Enable
#define SSTATUS_UIE (1L << 0)  // User Interrupt Enable

static inline uint64
r_sstatus()
{
    uint64 x;
    asm volatile("csrr %0, sstatus" : "=r" (x) );
    return x;
}

static inline void 
w_sstatus(uint64 x)
{
    asm volatile("csrw sstatus, %0" : : "r" (x));
}

#define SIE_SEIE (1L << 9) // external
#define SIE_STIE (1L << 5) // timer
#define SIE_SSIE (1L << 1) // software
static inline uint64
r_sie()
{
    uint64 x;
    asm volatile("csrr %0, sie" : "=r" (x) );
    return x;
}

static inline void 
w_sie(uint64 x)
{
    asm volatile("csrw sie, %0" : : "r" (x));
}

// machine-mode cycle counter
static inline uint64
r_time()
{
  uint64 x;
  asm volatile("csrr %0, time" : "=r" (x) );
  return x;
}

static inline void
w_satp(uint64 x)
{
    asm volatile("csrw satp, %0"::"r"(x));
}

static inline void
flush_tlb()
{
    asm volatile("sfence.vma zero, zero");
}

static inline uint64
r_satp()
{
    uint64 x;
    asm volatile("csrr %0, satp" : "=r" (x) );
    return x;
}

static inline void
enable_and_wfi()
{
    asm volatile("csrsi sstatus, 1 << 1; wfi");
}

static inline uint64
disable_and_store()
{
    uint64 x;
    asm volatile("csrci sstatus, 1 << 1" : "=r" (x) );
    return (x & (1 << 1));
}

static inline void
restore_sstatus(uint64 flags)
{
    asm volatile("csrs sstatus, %0" :: "r"(flags) );
}

#endif