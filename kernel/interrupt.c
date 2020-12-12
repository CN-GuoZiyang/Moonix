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
    // 初始化时钟中断
    extern void initTimer(); initTimer();
    printf("***** Init Interrupt *****\n");
}

void
breakpoint(Context *context)
{
    printf("Breakpoint at %p\n", context->sepc);
    // ebreak 指令长度 2 字节，返回后跳过该条指令
    context->sepc += 2;
}

void
supervisorTimer(Context *context)
{
    extern void tick();
    tick();
}

void
fault(Context *context, usize scause, usize stval)
{
    printf("Unhandled interrupt!\nscause\t= %psepc\t= %p\nstval\t= %p\n",
        scause,
        context->sepc,
        stval
    );
    panic("");
}

void
handleInterrupt(Context *context, usize scause, usize stval)
{
    switch (scause)
    {
    case BREAKPOINT:
        breakpoint(context);
        break;
    case SUPERVISOR_TIMER:
        supervisorTimer(context);
        break;
    default:
        fault(context, scause, stval);
        break;
    }
}