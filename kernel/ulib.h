#ifndef _ULIB_H
#define _ULIB_H

static inline void
userPutchar(usize c)
{
    register unsigned long a0 asm("a0") = (unsigned long)(c);
    register unsigned long a7 asm("a7") = (unsigned long)(64);
    asm volatile("ecall"::"r"(a0),"r"(a7):"memory");
}

#endif