#include "types.h"
#include "riscv.h"
#include "context.h"
#include "defs.h"

extern void __alltraps();

void
handle_trap(TrapFrame *tf) {
    printf("handle trap!\n");
    tf->sepc += 2;
}

void
init_interrupt()
{
    w_sscratch(0L);
    w_stvec((uint64)__alltraps);
    printf("---- setup interrupt ----\n");
}