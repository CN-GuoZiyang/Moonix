#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "context.h"

extern void __alltraps();
extern int TICKS;

void
breakpoint(TrapFrame *tf)
{
    printf("a breakpoint set %p\n", tf->sepc);
    tf->sepc += 2;
}

void
super_timer()
{
    clock_set_next_event();
    TICKS += 1;
    if(TICKS == 100) {
        TICKS = 0;
        printf("* 100 ticks *\n");
    }
    tick();
}

void
handle_syscall(TrapFrame *tf)
{
    tf->sepc += 4;
    uint64 ret = syscall(tf->x[17], (uint64[]){tf->x[10], tf->x[11], tf->x[12]}, tf);
    tf->x[10] = ret;
}

void
handle_trap(TrapFrame *tf)
{
    switch (tf->scause)
    {
    case 3L:
        breakpoint(tf);
        break;
    case 8L:
        handle_syscall(tf);
        break;
    case 5L|(1L<<63):
        super_timer();
        break;
    default:
        printf("scause %p, sepc %p, stval %p\n", tf->scause, tf->sepc, tf->stval);
        panic("Undefined trap!");
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