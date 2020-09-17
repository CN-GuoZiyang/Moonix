#include "types.h"
#include "riscv.h"
#include "defs.h"

void
trap_handler()
{
    uint64 scause = r_scause();
    uint64 sepc = r_sepc();
    printf("Trap: scause: %p, sepc: %p\n", scause, sepc);
    panic("Trap handled!");
}__attribute__ ((aligned (4)))

void
init_interrupt()
{
    w_sscratch(0L);
    w_stvec((uint64)trap_handler);
    printf("---- setup interrupt ----\n");
}