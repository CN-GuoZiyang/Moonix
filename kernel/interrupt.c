#include "types.h"
#include "sbi.h"
#include "context.h"

asm(".include \"kernel/interrupt.asm\"");

void
setNextTimer()
{
    usize time;
    asm volatile("csrr %0, time":"=r"(time));
    setTimer(time + 10000);
}

extern void switchToAnother();

void
handleInterrupt(usize scause)
{
    switch(scause)
    {
        case 5L | (1L << 63):
            setNextTimer();
            switchToAnother();
            break;
        default:
            consolePutchar('C');
            break;
    }
}

void
initInterrupt()
{
    // 设置 stvec 寄存器
    extern void __interrupt();
    usize stvec = (usize)__interrupt | 0x0;
    asm volatile("csrw stvec, %0"::"r" (stvec));

    // 时钟中断使能
    usize sie;
    asm volatile("csrr %0, sie":"=r" (sie));
    sie |= (1L << 5);
    asm volatile("csrw sie, %0"::"r" (sie));

    // S-Mode 全局中断使能
    usize sstatus;
    asm volatile("csrr %0, sstatus":"=r"(sstatus));
    sstatus |= (1L << 1);
    asm volatile("csrw sstatus, %0"::"r"(sstatus));
}