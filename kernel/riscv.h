#ifndef _RISCV_H
#define _RISCV_H

#include "types.h"

#define MODE_DIRECT 0x0
#define MODE_VECTOR 0x1
// stvec 寄存器设置中断处理函数和处理模式
static inline void 
w_stvec(usize x)
{
    asm volatile("csrw stvec, %0" : : "r" (x));
}


#define SIE_SEIE (1L << 9) // 外部中断
#define SIE_STIE (1L << 5) // 时钟中断
#define SIE_SSIE (1L << 1) // 软件中断
// SIE 寄存器设置一些中断使能
static inline usize
r_sie()
{
    usize x;
    asm volatile("csrr %0, sie" : "=r" (x) );
    return x;
}

static inline void 
w_sie(usize x)
{
    asm volatile("csrw sie, %0" : : "r" (x));
}

#define SSTATUS_SIE (1L << 1)  // 监管者模式中断使能
#define SSTATUS_UIE (1L << 0)  // 用户模式中断使能
// 监管者模式状态寄存器
static inline usize
r_sstatus()
{
    usize x;
    asm volatile("csrr %0, sstatus" : "=r" (x) );
    return x;
}

static inline void 
w_sstatus(usize x)
{
    asm volatile("csrw sstatus, %0" : : "r" (x));
}

// 读取硬件时钟
static inline usize
r_time()
{
    usize x;
    asm volatile("csrr %0, time" : "=r" (x) );
    return x;
}

#endif