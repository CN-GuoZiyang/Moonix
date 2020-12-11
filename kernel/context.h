#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "types.h"

// 中断上下文，scause 和 stval 作为参数传递，无需保存
typedef struct
{
    // 32个通用寄存器
    usize x[32];
    // S-Mode 状态寄存器
    usize sstatus;
    // 中断返回地址
    usize sepc;
} Context;

#endif