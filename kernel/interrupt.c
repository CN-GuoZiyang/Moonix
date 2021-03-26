#include "types.h"
#include "sbi.h"
#include "context.h"

asm(".include \"kernel/interrupt.asm\"");

void
setNextTimer()
{
    usize time;
    asm volatile("csrr %0, time":"=r"(time));
    // 此处时钟中断粒度过小可能导致无法输出
    setTimer(time + 50000);
}

void
handleSyscall(InterruptContext *context)
{
    context->sepc += 4;
    switch(context->x[17]) {
        case 64:
            consolePutchar(context->x[10]);
            break;
        default:
            consolePutchar('C');
            break;
    }
}

extern void switchToAnother();

void
handleInterrupt(InterruptContext *context, usize scause)
{
    switch(scause)
    {
        case 8L:
            handleSyscall(context);
            break;
        // 时钟中断发生时，scause 寄存器会被设置为如下值
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
    // 设置 stvec 寄存器，设置中断入口地址
    extern void __interrupt();
    // 0x0 将中断跳转设置为 Direct MODE
    usize stvec = (usize)__interrupt | 0x0;
    asm volatile("csrw stvec, %0"::"r"(stvec));

    // 时钟中断使能
    usize sie;
    asm volatile("csrr %0, sie":"=r"(sie));
    sie |= (1L << 5);
    asm volatile("csrw sie, %0"::"r"(sie));

    // S-Mode 全局中断使能
    usize sstatus;
    asm volatile("csrr %0, sstatus":"=r"(sstatus));
    sstatus |= (1L << 1);
    asm volatile("csrw sstatus, %0"::"r"(sstatus));
}