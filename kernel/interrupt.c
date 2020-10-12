#include "types.h"
#include "riscv.h"
#include "context.h"
#include "defs.h"

extern void __alltraps();
uint64 TICKS = 0;

void
super_timer()
{
    set_next_clock();
    TICKS += 1;
    if(TICKS == 100) {
        TICKS = 0;
        printf("* 100 ticks *\n");
    }
    tick();
}

void
handle_breakpoint(TrapFrame *tf)
{
    printf("A breakpoint is set at %p\n", tf->sepc);
    tf->sepc += 2;
}

void
handle_trap(TrapFrame *tf)
{
    switch (tf->scause)
    {
    case 3:
        handle_breakpoint(tf);
        break;
    case (1L<<63 | 5L):
        super_timer();
        break;
    default:
        printf("unknown trap %p, sepc %p, stval %p\n", tf->scause, tf->sepc, tf->stval);
        panic("");
        break;
    }
}

void
init_interrupt()
{
    w_sscratch(0L);
    w_stvec((uint64)__alltraps);
    w_sstatus(r_sstatus() | SSTATUS_SIE);
    printf("---- setup interrupt ----\n");
}