#include "types.h"
#include "def.h"
#include "riscv.h"

static const usize INTERVAL = 100000;
static usize TICKS = 0;

void setTimeout();

void
initTimer()
{
    // 时钟中断使能
    w_sie(r_sie() | SIE_STIE);
    // 监管者模式中断使能
    w_sstatus(r_sstatus() | SSTATUS_SIE);
    // 设置第一次时钟中断
    setTimeout();
}

void
setTimeout()
{
    // 设置下一次时钟时间为当前时间 + INTERVAL
    setTimer(r_time() + INTERVAL);
}

void
tick()
{
    setTimeout();
    TICKS += 1;
    if(TICKS % 100 == 0) {
        printf("** %d ticks **\n", TICKS);
    }
}