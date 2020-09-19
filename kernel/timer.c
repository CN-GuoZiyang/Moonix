#include "types.h"
#include "riscv.h"
#include "sbi.h"
#include "defs.h"

uint64 TICKS = 0;
uint64 TIMEBASE = 100000;

void
set_next_clock()
{
    SBI_ECALL_1(SBI_SET_TIMER, r_time() + TIMEBASE);
}

void
init_timer()
{
    TICKS = 0;
    w_sie(r_sie() | SIE_STIE);
    set_next_clock();
    printf("---- setup timer ----\n");
}