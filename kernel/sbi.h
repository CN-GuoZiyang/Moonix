#ifndef _SBI_H
#define _SBI_H

static inline void
consolePutchar(usize c)
{
    register unsigned long a0 asm("a0") = (unsigned long)(c);
    register unsigned long a7 asm("a7") = (unsigned long)(0x1);
    asm volatile("ecall"::"r"(a0),"r"(a7):"memory");
}

static inline void
setTimer(usize time)
{
    register unsigned long a0 asm("a0") = (unsigned long)(time);
    register unsigned long a7 asm("a7") = (unsigned long)(0x0);
    asm volatile("ecall"::"r"(a0),"r"(a7):"memory");
}

#endif