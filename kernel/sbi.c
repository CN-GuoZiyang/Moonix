/*
 *  kernel/sbi.c
 *  
 *  (C) 2021  Ziyang Guo
 */

/*
 * sbi.c 对 OpenSBI 提供的服务进行简单的包装
 */

#include "types.h"
#include "def.h"
#include "sbi.h"

void
consolePutchar(usize c)
{
    SBI_ECALL_1(SBI_CONSOLE_PUTCHAR, c);
}

usize
consoleGetchar()
{
    return SBI_ECALL_0(SBI_CONSOLE_GETCHAR);
}

void
shutdown()
{
    SBI_ECALL_0(SBI_SHUTDOWN);
    while(1) {}
}

void
setTimer(usize time)
{
    SBI_ECALL_1(SBI_SET_TIMER, time);
}