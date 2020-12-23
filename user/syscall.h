#ifndef _SYSCALL_H
#define _SYSCALL_H

typedef enum {
    Read = 63,
    Write = 64,
    Exit = 93,
} SyscallId;

#define sys_call(__num, __a0, __a1, __a2, __a3)                          \
({                                                                  \
    register unsigned long a0 asm("a0") = (unsigned long)(__a0);    \
    register unsigned long a1 asm("a1") = (unsigned long)(__a1);    \
    register unsigned long a2 asm("a2") = (unsigned long)(__a2);    \
    register unsigned long a3 asm("a3") = (unsigned long)(__a3);    \
    register unsigned long a7 asm("a7") = (unsigned long)(__num);   \
    asm volatile("ecall"                                            \
                : "+r"(a0)                                          \
                : "r"(a1), "r"(a2), "r"(a3), "r"(a7)                         \
                : "memory");                                        \
    a0;                                                             \
})

#define sys_read(__a0, __a1, __a2) sys_call(Read, __a0, __a1, __a2, 0)
#define sys_write(__a0) sys_call(Write, __a0, 0, 0, 0)
#define sys_exit(__a0) sys_call(Exit, __a0, 0, 0, 0)

#endif