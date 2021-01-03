#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "types.h"

/* 中断发生前后的程序上下文 */
typedef struct
{
    usize x[32];            /* 32 个通用寄存器 */
    usize sstatus;          /* S-Mode 状态寄存器 */
    usize sepc;             /* 中断处理结束后的返回地址 */
} InterruptContext;

/* 线程切换前后的线程上下文 */
typedef struct {
    usize ra;               /* ra 寄存器，保存线程当前的运行位置 */
    usize satp;             /* satp 寄存器，保存线程使用的三级页表基址 */
    usize s[12];            /* 被调用者保存的 12 个通用寄存器 */
    InterruptContext ic;    /* 借助中断处理结束后的恢复机制，对线程进行初始化，这部分不会在切换时保存在栈上 */
} ThreadContext;

#endif