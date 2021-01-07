#ifndef _SYSCALL_H
#define _SYSCALL_H

typedef enum {
    Shutdown = 13,
    LsDir = 20,
    CdDir = 21,
    Pwd = 22,
    Open = 56,
    Close = 57,
    Read = 63,
    Write = 64,
    Exit = 93,
    Exec = 221,
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

#define sys_shut() sys_call(Shutdown, 0, 0, 0, 0)
#define sys_lsdir(__a0, __a1) sys_call(LsDir, __a0, __a1, 0, 0)
#define sys_cddir(__a0, __a1) sys_call(CdDir, __a0, __a1, 0, 0)
#define sys_pwd(__a0) sys_call(Pwd, __a0, 0, 0, 0)
#define sys_open(__a0) sys_call(Open, __a0, 0, 0, 0)
#define sys_close(__a0) sys_call(Close, __a0, 0, 0, 0)
#define sys_read(__a0, __a1, __a2) sys_call(Read, __a0, __a1, __a2, 0)
#define sys_write(__a0) sys_call(Write, __a0, 0, 0, 0)
#define sys_exit(__a0) sys_call(Exit, __a0, 0, 0, 0)
#define sys_exec(__a0, __a1) sys_call(Exec, __a0, __a1, 0, 0)

#endif