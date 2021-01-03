/*
 *  kernel/interrupt.c
 *  
 *  (C) 2021  Ziyang Guo
 */

#include "types.h"
#include "context.h"
#include "def.h"
#include "riscv.h"
#include "interrupt.h"
#include "consts.h"
#include "stdin.h"

asm(".include \"kernel/interrupt.asm\"");

/*
 * OpenSBI默认会关闭所有的外部中断和串口设备中断（键盘中断），以防止初始化过程被打断
 * 这里需要手动打开
 * 写法来源：https://github.com/rcore-os/rCore/blob/3ac4d7a607dbe81167f5d6ad799bc91682ab9f7d/kernel/src/arch/riscv/board/virt/mod.rs
 */
void
initExternalInterrupt()
{
    *(uint32 *)(0x0C002080 + KERNEL_MAP_OFFSET) = 0x400U;
    *(uint32 *)(0x0C000028 + KERNEL_MAP_OFFSET) = 0x7U;
    *(uint32 *)(0x0C201000 + KERNEL_MAP_OFFSET) = 0x0U;
}

void
initSerialInterrupt()
{
    *(uint8 *)(0x10000001 + KERNEL_MAP_OFFSET) = 0x01U;
    *(uint8 *)(0x10000004 + KERNEL_MAP_OFFSET) = 0x0bU;
}

void
initInterrupt()
{
    /* 
     * 设置 stvec 寄存器
     * 设置中断处理函数和处理模式
     */
    extern void __interrupt();
    w_stvec((usize)__interrupt | MODE_DIRECT);

    /* 开启外部中断 */
    w_sie(r_sie() | SIE_SEIE);

    /* 打开 OpenSBI 的外部中断响应和串口设备响应 */
    initExternalInterrupt();
    initSerialInterrupt();

    printf("***** Init Interrupt *****\n");
}

/* 
 * 处理外部中断
 * 由于目前外部设备只打开了串口设备
 * 所以只需要处理键盘中断
 */
void
external()
{
    usize ret = consoleGetchar();
    /* 
     * 调用 SBI 接口获得输入的字符，可能出现错误
     * 所有获取到的字符都被存入标准输入缓冲区
     */
    if(ret != -1) {
        char ch = (char)ret;
        if(ch == '\r') {
            pushChar('\n');
        } else {
            pushChar(ch);
        }
    }
}

/* 
 * 断点中断
 * 输出断点信息并跳转到下一条指令
 */
void
breakpoint(InterruptContext *context)
{
    printf("Breakpoint at %p\n", context->sepc);
    /* ebreak 指令长度 2 字节 */
    context->sepc += 2;
}

/* 
 * 处理来自 U-Mode 的系统调用
 * 返回值存储在 x10（a0）寄存器
 */
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

/*
 * 时钟中断，主要用于调度
 * 设置下一次时钟中断时间并通知调度器检查当前线程时间片
 */
void
supervisorTimer()
{
    extern void tick(); tick();
    extern void tickCPU(); tickCPU();
}

/* 
 * 未知中断
 * 直接打印信息并关机
 */
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