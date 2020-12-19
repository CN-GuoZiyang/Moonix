#include "types.h"
#include "context.h"
#include "def.h"
#include "riscv.h"
#include "interrupt.h"

asm(".include \"kernel/interrupt.asm\"");

void
initInterrupt()
{
    // 设置 stvec 寄存器，设置中断处理函数和处理模式
    extern void __interrupt();
    w_stvec((usize)__interrupt | MODE_DIRECT);
    printf("***** Init Interrupt *****\n");
}

// 断点中断，打印信息并跳转到下一条指令
void
breakpoint(InterruptContext *context)
{
    printf("Breakpoint at %p\n", context->sepc);
    // ebreak 指令长度 2 字节，返回后跳过该条指令
    context->sepc += 2;
}

// 时钟中断，主要用于调度
void
supervisorTimer()
{
    extern void tick(); tick();
    extern void tickCPU(); tickCPU();
}

// 未知错误直接打印信息并关机
void
fault(InterruptContext *context, usize scause, usize stval)
{
    printf("Unhandled interrupt!\nscause\t= %p\nsepc\t= %p\nstval\t= %p\n",
        scause,
        context->sepc,
        stval
    );
    panic("");
}

void
handleInterrupt(InterruptContext *context, usize scause, usize stval)
{
    switch (scause)
    {
    case BREAKPOINT:
        breakpoint(context);
        break;
    case SUPERVISOR_TIMER:
        supervisorTimer();
        break;
    default:
        fault(context, scause, stval);
        break;
    }
}