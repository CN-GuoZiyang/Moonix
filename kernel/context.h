#ifndef _CONTEXT_H
#define _CONTEXT_H

// 程序中断上下文
typedef struct
{
    usize x[32];
    usize sstatus;
    usize sepc;
} InterruptContext;

// 线程运行上下文
typedef struct {
    usize s[12];
    usize ra;
    InterruptContext ic;
} ThreadContext;

#endif