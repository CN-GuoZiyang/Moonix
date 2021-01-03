/*
 *  kernel/timer.c
 *  
 *  (C) 2021  Ziyang Guo
 */

#include "types.h"
#include "def.h"
#include "riscv.h"

static const usize INTERVAL = 100000;   /* 时钟中断间隔 */

void setTimeout();

void
initTimer()
{
    /* 时钟中断使能 */
    w_sie(r_sie() | SIE_STIE);
    /* 允许 S-Mode 线程被中断打断 */
    w_sstatus(r_sstatus() | SSTATUS_SIE);
    /* 初始化第一次时钟中断 */
    setTimeout();
}

void
setTimeout()
{
    /* 设置下一次时钟时间为当前时间 + INTERVAL */
    setTimer(r_time() + INTERVAL);
}

void
tick()
{
    setTimeout();
}