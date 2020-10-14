#ifndef _SYSCALL_H
#define _SYSCALL_H

typedef enum syscallid {
    Write = 64,
    Exit = 93,
} SyscallId;

inline uint64
sys_call(SyscallId syscall_id, uint64 arg0, uint64 arg1, uint64 arg2, uint64 arg3)
{
    uint64 id = syscall_id;
    uint64 ret;
    asm volatile("ecall"
                : "={x10}"(ret)
                : "{x17}"(id), "{x10}"(arg0), "x11"(arg1), "{x12}"(arg2), "{x13}"(arg3)
                : "memory");
    return ret;
}

uint64
sys_write(uint8 ch)
{
    return sys_call(Write, (uint64)ch, 0, 0, 0);
}

void
sys_exit(uint64 code)
{
    sys_call(Exit, code, 0, 0, 0);
    while(1) {}
}

#endif