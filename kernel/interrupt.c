#include "types.h"
#include "context.h"
#include "def.h"
#include "riscv.h"
#include "interrupt.h"
#include "consts.h"
#include "stdin.h"

asm(".include \"kernel/interrupt.asm\"");

void
initExternalInterrupt()
{
    *(uint32 *)(0x0C002080 + KERNEL_MAP_OFFSET) = 1 << 0xa;
    *(uint32 *)(0x0C000028 + KERNEL_MAP_OFFSET) = 0x7U;
    *(uint32 *)(0x0C201000 + KERNEL_MAP_OFFSET) = 0x0U;
}

void
initSerialInterrupt()
{
    *(uint8 *)(0x10000004 + KERNEL_MAP_OFFSET) = 0x0bU;
    *(uint8 *)(0x10000001 + KERNEL_MAP_OFFSET) = 0x01U;
}

void
initInterrupt()
{
    // 设置 stvec 寄存器，设置中断处理函数和处理模式
    extern void __interrupt();
    w_stvec((usize)__interrupt | MODE_DIRECT);

    // 开启外部中断
    w_sie(r_sie() | SIE_SEIE);

    // 打开 OpenSBI 的外部中断响应
    initExternalInterrupt();
    initSerialInterrupt();

    printf("***** Init Interrupt *****\n");
}

// 处理外部中断
void
external()
{
    // 目前只处理串口中断
    usize ret = consoleGetchar();
    if(ret != -1) {
        char ch = (char)ret;
        if(ch == '\r') {
            pushChar('\n');
        } else {
            pushChar(ch);
        }
    }
}

// 断点中断，打印信息并跳转到下一条指令
void
breakpoint(InterruptContext *context)
{
    printf("Breakpoint at %p\n", context->sepc);
    // ebreak 指令长度 2 字节，返回后跳过该条指令
    context->sepc += 2;
}

// 处理来自 U-Mode 的系统调用
void
handleSyscall(InterruptContext *context)
{
    context->sepc += 4;
    extern usize syscall(usize id, usize args[3], InterruptContext *context);
    usize ret = syscall(
        context->x[17],
        (usize[]){context->x[10], context->x[11], context->x[12]},
        context
    );
    context->x[10] = ret;
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
    case USER_ENV_CALL:
        handleSyscall(context);
        break;
    case SUPERVISOR_TIMER:
        supervisorTimer();
        break;
    case SUPERVISOR_EXTERNAL:
        external();
        break;
    default:
        fault(context, scause, stval);
        break;
    }
}